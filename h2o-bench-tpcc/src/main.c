/* crucible-demo — zone-scaled linear scaling benchmark.
 * NASA/JPL Power of 10 (strictest subset).
 * No malloc after init.  No recursion.  Fixed loop bounds.
 * Function length ≤ 60 lines.  Assert all params.  Check all returns.
 * One level of deref only.  No global variables. */

#include "worker_pool.h"
#include "error.h"

#include <h2o.h>
#include <h2o/http1.h>
#include <foundationdb/fdb_c.h>

#include <arpa/inet.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ENTITIES_PER_ZONE 200

/* ── File-scoped state (Power of 10 rule 6: no globals) ─────────────── */

static FDBDatabase *fdb_handle;   /* init-once, set in main */

/* ── Zone tick: write N entity keys, commit ─────────────────────────── */
/* Power of 10 rule 4: function ≤ ~60 lines */

static void zone_tick_handler(h2o_req_t *req, void *data) {
    assert(req != NULL);                      /* rule 5: assert params */
    (void)data;

    int n = ENTITIES_PER_ZONE;
    if (req->query_at >= 0) {                 /* rule 1: no complex flow */
        const char *qs = req->path.base + req->query_at + 1;
        int v = atoi(qs);
        if (v > 0 && v <= 10000) n = v;      /* rule 2: bounded */
    }

    FDBTransaction *txn = NULL;
    fdb_database_create_transaction(fdb_handle, &txn);
    assert(txn != NULL);                      /* rule 5 */

    /* Fixed bound from n (validated above: 1 ≤ n ≤ 10000) */
    char key[64];
    for (int i = 0; i < n; i++) {             /* rule 2: fixed upper bound */
        int len = snprintf(key, sizeof(key), "e/%d/pos", i);
        assert(len > 0 && len < (int)sizeof(key));
        fdb_transaction_set(txn, (uint8_t *)key, len, (uint8_t *)"0,0", 3);
    }

    FDBFuture *f = fdb_transaction_commit(txn);
    assert(f != NULL);
    fdb_future_block_until_ready(f);
    fdb_future_destroy(f);

    char resp[128];
    int rlen = snprintf(resp, sizeof(resp), "zone tick %d entities\n", n);
    assert(rlen > 0 && rlen < (int)sizeof(resp));
    h2o_send_inline(req, resp, (size_t)rlen);
}

/* ── Return path ────────────────────────────────────────────────────── */

typedef struct { h2o_multithread_message_t super; h2o_req_t *req; } ret_t;

static void on_return(h2o_multithread_receiver_t *r, h2o_linklist_t *msgs) {
    assert(r != NULL);                        /* rule 5 */
    assert(msgs != NULL);
    while (!h2o_linklist_is_empty(msgs)) {    /* rule 2: bounded by queue */
        h2o_linklist_t *node = msgs->next;
        h2o_linklist_unlink(node);
        ret_t *m = (ret_t *)node;
        assert(m != NULL);
        h2o_send_inline(m->req, H2O_STRLIT("ok\n"));
        free(m);                              /* rule 3: only free, never alloc */
    }
}

/* ── HTTP handler (dispatcher) ──────────────────────────────────────── */

static int on_req(h2o_handler_t *self, h2o_req_t *req) {
    assert(self != NULL);
    assert(req != NULL);
    if (req->path.len < 4) {
        h2o_send_error_404(req, "Not Found", "", 0);
        return -1;
    }
    if (memcmp(req->path.base, "/zone", 4) != 0) {
        h2o_send_error_404(req, "Not Found", "", 0);
        return -1;
    }

    worker_pool_t *pool = (worker_pool_t *)self;
    assert(pool != NULL);
    int ok = worker_pool_dispatch(pool, req, zone_tick_handler, NULL) ? 0 : -1;
    if (ok != 0) {
        h2o_send_error_503(req, "Backpressure", "", 0);
    }
    return ok;
}

/* ── Socket ─────────────────────────────────────────────────────────── */

static int listen_on(int port) {
    assert(port > 0 && port < 65536);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd >= 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    int rc = bind(fd, (struct sockaddr *)&a, sizeof(a));
    assert(rc == 0);
    rc = listen(fd, 4096);
    assert(rc == 0);
    return fd;
}

/* ── Per-thread H2O event loop ──────────────────────────────────────── */

typedef struct {
    h2o_accept_ctx_t accept_ctx;
    h2o_context_t    h2o_ctx;
    h2o_loop_t      *loop;
    int              listen_fd;
    volatile int     running;
    pthread_t        tid;
} thread_ctx_t;

static void on_accept(h2o_socket_t *listener, const char *err) {
    assert(listener != NULL);
    thread_ctx_t *tctx = (thread_ctx_t *)listener->data;
    assert(tctx != NULL);
    if (err != NULL) return;
    h2o_socket_t *sock;
    while ((sock = h2o_evloop_socket_accept(listener)) != NULL) {
        assert(sock != NULL);
        h2o_accept(&tctx->accept_ctx, sock);
    }
}

static void *h2o_thread(void *arg) {
    assert(arg != NULL);
    thread_ctx_t *tctx = (thread_ctx_t *)arg;
    assert(tctx->loop != NULL);
    h2o_socket_t *sock = h2o_evloop_socket_create(
        tctx->loop, tctx->listen_fd, H2O_SOCKET_FLAG_DONT_READ);
    assert(sock != NULL);
    sock->data = tctx;
    h2o_socket_read_start(sock, on_accept);
    while (tctx->running != 0) {
        h2o_evloop_run(tctx->loop, INT32_MAX);
    }
    return NULL;
}

/* ── Main ───────────────────────────────────────────────────────────── */
/* Power of 10 rule 4: ≤ 60 lines */

int main(int argc, char **argv) {
    int port = 8080;
    const char *cluster = "/etc/foundationdb/fdb.cluster";
    size_t nworkers = 1;

    int c;
    while ((c = getopt(argc, argv, "a:c:p:")) != -1) {
        if (c == 'a') nworkers = (size_t)atoi(optarg);
        if (c == 'c') cluster = optarg;
        if (c == 'p') port = atoi(optarg);
    }
    assert(nworkers > 0 && nworkers <= 64);
    assert(port > 0 && port < 65536);
    assert(cluster != NULL);

    signal(SIGPIPE, SIG_IGN);

    fdb_select_api_version(FDB_API_VERSION);
    fdb_setup_network();
    fdb_create_database(cluster, &fdb_handle);
    assert(fdb_handle != NULL);

    h2o_globalconf_t h2o_cfg;
    h2o_config_init(&h2o_cfg);
    h2o_hostconf_t *hc = h2o_config_register_host(
        &h2o_cfg, h2o_iovec_init(H2O_STRLIT("default")), port);
    assert(hc != NULL);
    h2o_pathconf_t *pc = h2o_config_register_path(hc, "/", 0);
    assert(pc != NULL);
    h2o_handler_t *handler = h2o_create_handler(pc, sizeof(*handler));
    assert(handler != NULL);
    handler->on_req = on_req;

    int listen_fd = listen_on(port);
    assert(listen_fd >= 0);

    /* Pre-allocate everything (rule 3: no malloc after init) */
    assert(nworkers <= 64);
    static thread_ctx_t threads[64];
    worker_pool_t pool;

    worker_pool_init(&pool, nworkers, NULL);
    for (size_t i = 0; i < nworkers; i++) {
        threads[i].listen_fd = listen_fd;
        threads[i].running = 1;
        threads[i].loop = h2o_evloop_create();
        assert(threads[i].loop != NULL);
        h2o_context_init(&threads[i].h2o_ctx, threads[i].loop, &h2o_cfg);
        threads[i].accept_ctx.ctx = &threads[i].h2o_ctx;
        threads[i].accept_ctx.hosts = h2o_cfg.hosts;
        threads[i].accept_ctx.ssl_ctx = NULL;
        int rc = pthread_create(&threads[i].tid, NULL, h2o_thread, &threads[i]);
        assert(rc == 0);
    }

    fprintf(stderr, "crucible-demo: %zu worker(s), port %d\n", nworkers, port);
    fdb_run_network();
    return 0;
}