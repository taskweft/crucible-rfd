/* crucible-demo — linear scaling proof of concept.
 * Based on h2o-bench-tpcc architecture (subtree).
 * libh2o + FDB + SPSC worker pool.  One /ping endpoint.
 * N workers = N× throughput. */

#include "spsc_ring.h"
#include "worker_pool.h"
#include "error.h"

#include <h2o.h>
#include <h2o/http1.h>
#include <fdb_c.h>

#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_PORT 8080

typedef struct {
    h2o_accept_ctx_t accept_ctx;
    h2o_context_t h2o_ctx;
    h2o_loop_t *loop;
    int listen_fd;
    FDBDatabase *fdb;
    pthread_t tid;
    volatile int running;
} thread_ctx_t;

/* ── FDB transaction: set + get the tick counter ────────────────────── */

static void ping_handler(h2o_req_t *req, void *data) {
    FDBDatabase *fdb = (FDBDatabase *)data;
    FDBTransaction *txn;
    fdb_database_create_transaction(fdb, &txn);

    const char *key = "tick";
    FDBFuture *set = fdb_transaction_set(txn, (uint8_t *)key, 4, (uint8_t *)"1", 1);
    fdb_future_destroy(set);

    FDBFuture *get = fdb_transaction_get(txn, (uint8_t *)key, 4, 0);
    fdb_future_block_until_ready(get);
    fdb_future_destroy(get);
    fdb_transaction_commit(txn);

    h2o_iovec_t body = h2o_strdup(&req->pool, "ok\n", 3);
    h2o_send_inline(req, body.base, body.len);
}

/* ── Worker return path ─────────────────────────────────────────────── */

typedef struct { h2o_multithread_message_t super; h2o_req_t *req; } ret_t;

static void on_return(h2o_multithread_receiver_t *r, h2o_linklist_t *msgs) {
    (void)r;
    while (!h2o_linklist_is_empty(msgs)) {
        h2o_linklist_t *node = msgs->next;
        h2o_linklist_unlink(node);
        ret_t *m = (ret_t *)node;
        h2o_iovec_t body = h2o_strdup(&m->req->pool, "ok\n", 3);
        h2o_send_inline(m->req, body.base, body.len);
        free(m);
    }
}

/* ── Per-worker runner ──────────────────────────────────────────────── */

typedef struct { void (*handler)(h2o_req_t *, void *); h2o_req_t *req; void *data; } work_t;

static void *worker_thread(void *arg) {
    worker_t *w = (worker_t *)arg;
    while (atomic_load(&w->running)) {
        work_t *item = (work_t *)spsc_ring_pop(&w->ring);
        if (item) {
            item->handler(item->req, item->data);
            ret_t *m = malloc(sizeof(*m));
            m->req = item->req;
            h2o_multithread_send_message(&w->receiver, &m->super);
            free(item);
        } else { sched_yield(); }
    }
    return NULL;
}

/* ── HTTP handler ───────────────────────────────────────────────────── */

static int on_req(h2o_handler_t *self, h2o_req_t *req) {
    (void)self;
    if (req->path.len == 5 && memcmp(req->path.base, "/ping", 5) == 0) {
        WorkerPool *pool = self;
        work_t *w = malloc(sizeof(*w));
        w->req = req; w->handler = ping_handler; w->data = NULL;
        if (!worker_pool_dispatch(pool, req, NULL, NULL)) {
            free(w);
            h2o_send_error_503(req, "Backpressure", "", 0);
        }
        return 0;
    }
    h2o_send_error_404(req, "Not Found", "", 0);
    return 0;
}

/* ── Socket ─────────────────────────────────────────────────────────── */

static int listen_on(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons(port) };
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) < 0) { perror("bind"); return -1; }
    if (listen(fd, 4096) < 0) { perror("listen"); return -1; }
    return fd;
}

/* ── Per-thread H2O event loop ──────────────────────────────────────── */

static void on_accept(h2o_socket_t *listener, const char *err) {
    thread_ctx_t *tctx = (thread_ctx_t *)listener->data;
    if (err) return;
    h2o_socket_t *sock;
    while ((sock = h2o_evloop_socket_accept(listener)) != NULL)
        h2o_accept(&tctx->accept_ctx, sock);
}

static void *h2o_thread(void *arg) {
    thread_ctx_t *tctx = (thread_ctx_t *)arg;
    h2o_socket_t *sock = h2o_evloop_socket_create(tctx->loop, tctx->listen_fd,
                                                    H2O_SOCKET_FLAG_DONT_READ);
    sock->data = tctx;
    h2o_socket_read_start(sock, on_accept);
    while (tctx->running) h2o_evloop_run(tctx->loop, INT32_MAX);
    return NULL;
}

/* ── Main ───────────────────────────────────────────────────────────── */

static FDBDatabase *fdb_global_ptr;

int main(int argc, char **argv) {
    int port = DEFAULT_PORT;
    const char *cluster = "/etc/foundationdb/fdb.cluster";
    size_t nworkers = 1;

    int c;
    while ((c = getopt(argc, argv, "a:c:p:")) != -1) {
        if (c == 'a') nworkers = (size_t)atoi(optarg);
        if (c == 'c') cluster = optarg;
        if (c == 'p') port = atoi(optarg);
    }

    signal(SIGPIPE, SIG_IGN);

    /* FDB */
    fdb_select_api_version(FDB_API_VERSION);
    fdb_setup_network();
    fdb_create_database(cluster, &fdb_global_ptr);
    fprintf(stderr, "FDB connected\n");

    /* H2O config + handler (route /ping) */
    h2o_globalconf_t h2o_cfg;
    h2o_config_init(&h2o_cfg);
    h2o_hostconf_t *hc = h2o_config_register_host(
        &h2o_cfg, h2o_iovec_init(H2O_STRLIT("default")), port);
    h2o_pathconf_t *pc = h2o_config_register_path(hc, "/", 0);
    h2o_handler_t *handler = h2o_create_handler(pc, sizeof(*handler));
    handler->on_req = on_req;

    int listen_fd = listen_on(port);
    if (listen_fd < 0) return 1;

    fprintf(stderr, "listening on port %d, %zu worker(s)\n", port, nworkers);

    /* Worker pool */
    WorkerPool pool;
    worker_pool_init(&pool, nworkers, NULL);
    for (size_t i = 0; i < nworkers; i++)
        pool.workers[i].data = fdb_global_ptr;

    /* One H2O event loop per worker (SO_REUSEPORT) */
    thread_ctx_t *threads = calloc(nworkers, sizeof(*threads));
    for (size_t i = 0; i < nworkers; i++) {
        thread_ctx_t *t = &threads[i];
        t->listen_fd = listen_fd;
        t->running = 1;
        t->fdb = fdb_global_ptr;
        t->loop = h2o_evloop_create();
        h2o_context_init(&t->h2o_ctx, t->loop, &h2o_cfg);
        t->accept_ctx.ctx = &t->h2o_ctx;
        t->accept_ctx.hosts = h2o_cfg.hosts;
        t->accept_ctx.ssl_ctx = NULL;
        pthread_create(&t->tid, NULL, h2o_thread, t);
    }

    /* Block on FDB network thread */
    fdb_run_network();
    return 0;
}