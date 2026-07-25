/* crucible-demo — linear scaling proof of concept.
 * libh2o + FDB + SPSC worker pool.  One /ping endpoint.
 * N workers = N× throughput.  That's it. */

#include "spsc_ring.h"

#include <h2o.h>
#include <h2o/http1.h>
#include <fdb_c.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SPSC_CAP 4096
#define MAX_WORKERS 64

/* ── Work item ─────────────────────────────────────────────────────── */

typedef struct {
    h2o_req_t *req;
    void (*handler)(h2o_req_t *, void *);
    void *data;
} work_t;

/* ── Per-worker state ──────────────────────────────────────────────── */

typedef struct {
    spsc_ring_t        ring;
    void              *slots[SPSC_CAP];
    pthread_t          thread;
    FDBDatabase       *fdb;
    h2o_multithread_receiver_t receiver;
    h2o_multithread_queue_t   *return_q;
    volatile int       running;
} worker_t;

/* ── Globals ───────────────────────────────────────────────────────── */

static worker_t  workers[MAX_WORKERS];
static size_t    nworkers;
static _Atomic size_t rr;  /* round-robin counter */

/* ── FDB transaction: set tick counter, read it back ───────────────── */

static void ping_handler(h2o_req_t *req, void *data) {
    FDBDatabase *fdb = (FDBDatabase *)data;
    FDBTransaction *txn;
    fdb_database_create_transaction(fdb, &txn);

    const char *key = "tick";
    FDBFuture *set = fdb_transaction_set(txn, (uint8_t *)key, 4,
                                          (uint8_t *)"1", 1);
    fdb_future_destroy(set);

    FDBFuture *get = fdb_transaction_get(txn, (uint8_t *)key, 4, 0);
    fdb_future_block_until_ready(get);

    const uint8_t *val; int vlen;
    fdb_future_get_value(get, &vlen, &val, NULL);
    fdb_future_destroy(get);
    fdb_transaction_commit(txn);

    h2o_iovec_t body = h2o_strdup(&req->pool, "ok\n", 3);
    h2o_send_inline(req, body.base, body.len);
}

/* ── Return path: worker → H2O event loop ─────────────────────────── */

typedef struct { h2o_multithread_message_t super; h2o_req_t *req; } ret_t;

static void on_return(h2o_multithread_receiver_t *r, h2o_linklist_t *msgs) {
    (void)r;
    while (!h2o_linklist_is_empty(msgs)) {
        h2o_linklist_t *node = msgs->next;
        h2o_linklist_unlink(node);
        ret_t *m = (ret_t *)node;
        ping_handler(m->req, NULL);
        free(m);
    }
}

/* ── Worker thread ─────────────────────────────────────────────────── */

static void *worker_thread(void *arg) {
    worker_t *w = (worker_t *)arg;
    while (w->running) {
        work_t *item = spsc_ring_pop(&w->ring);
        if (item) {
            item->handler(item->req, w->fdb);
            ret_t *m = malloc(sizeof(*m));
            m->req = item->req;
            h2o_multithread_send_message(&w->receiver, &m->super);
            free(item);
        } else {
            sched_yield();
        }
    }
    return NULL;
}

/* ── HTTP handler ──────────────────────────────────────────────────── */

static int handler(h2o_handler_t *self, h2o_req_t *req) {
    if (req->path.len == 5 && memcmp(req->path.base, "/ping", 5) == 0) {
        size_t idx = atomic_fetch_add(&rr, 1) % nworkers;
        work_t *w = malloc(sizeof(*w));
        w->req = req; w->handler = ping_handler;
        if (!spsc_ring_push(&workers[idx].ring, w)) {
            h2o_send_error_503(req, "Backpressure", "", 0);
            free(w);
        }
        return 0;
    }
    h2o_send_error_404(req, "Not Found", "", 0);
    return 0;
}

/* ── Socket ────────────────────────────────────────────────────────── */

static int listen_on(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons(port) };
    bind(fd, (struct sockaddr *)&a, sizeof(a));
    listen(fd, 4096);
    return fd;
}

/* ── Main ──────────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    int port = 8080;
    const char *cluster = "/etc/foundationdb/fdb.cluster";
    nworkers = 1;

    for (int c; (c = getopt(argc, argv, "a:c:p:")) != -1;) {
        if (c == 'a') nworkers = atoi(optarg);
        if (c == 'c') cluster = optarg;
        if (c == 'p') port = atoi(optarg);
    }
    if (nworkers > MAX_WORKERS) nworkers = MAX_WORKERS;

    signal(SIGPIPE, SIG_IGN);

    /* FDB network */
    fdb_select_api_version(FDB_API_VERSION);
    fdb_setup_network();
    FDBDatabase *fdb;
    fdb_create_database(cluster, &fdb);

    /* H2O config */
    h2o_globalconf_t h2o_cfg;
    h2o_config_init(&h2o_cfg);
    h2o_hostconf_t *hc = h2o_config_register_host(
        &h2o_cfg, h2o_iovec_init(H2O_STRLIT("default")), port);
    h2o_pathconf_t *pc = h2o_config_register_path(hc, "/", 0);
    h2o_handler_t *h = h2o_create_handler(pc, sizeof(*h));
    h->on_req = handler;

    int listen_fd = listen_on(port);
    fprintf(stderr, "crucible-demo: %zu worker(s) on port %d\n", nworkers, port);

    /* Start workers */
    for (size_t i = 0; i < nworkers; i++) {
        fdb_database_create_transaction(fdb, NULL); /* warm */
        workers[i].fdb = fdb;
        workers[i].running = 1;
        spsc_ring_init(&workers[i].ring, workers[i].slots, SPSC_CAP);
        workers[i].return_q = h2o_multithread_create_queue(NULL);
        h2o_multithread_register_receiver(workers[i].return_q,
                                          &workers[i].receiver, on_return);
        pthread_create(&workers[i].thread, NULL, worker_thread, &workers[i]);
    }

    /* H2O event loops — one per worker via SO_REUSEPORT */
    h2o_loop_t *loops[MAX_WORKERS];
    pthread_t h2o_threads[MAX_WORKERS];

    for (size_t i = 0; i < nworkers; i++) {
        loops[i] = h2o_evloop_create();
        h2o_context_t *ctx = malloc(sizeof(*ctx));
        h2o_context_init(ctx, loops[i], &h2o_cfg);

        h2o_socket_t *sock = h2o_evloop_socket_create(loops[i], listen_fd,
            H2O_SOCKET_FLAG_DONT_READ);
        h2o_socket_read_start(sock, NULL);

        h2o_accept_ctx_t *actx = malloc(sizeof(*actx));
        *actx = (h2o_accept_ctx_t){ .ctx = ctx, .hosts = h2o_cfg.hosts };

        pthread_create(&h2o_threads[i], NULL,
            (void *(*)(void *))h2o_evloop_run, loops[i]);
    }

    fdb_run_network(); /* blocks until SIGINT */
    return 0;
}
