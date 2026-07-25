#!/bin/bash
set -euo pipefail

# Start FoundationDB
echo "Starting FoundationDB..."
if [ ! -f /etc/foundationdb/fdb.cluster ]; then
    echo "Docker: single" > /etc/foundationdb/fdb.cluster
fi
/usr/lib/foundationdb/fdbmonitor --conffile /etc/foundationdb/foundationdb.conf \
    --lockfile /var/log/foundationdb/fdbmonitor.pid &

# Wait for FDB
for i in $(seq 1 30); do
    if fdbcli --exec "status minimal" --timeout 2 2>/dev/null | grep -q "committed"; then
        echo "FDB ready after ${i}s"
        break
    fi
    if [ "$i" = "30" ]; then echo "FDB failed to start"; exit 1; fi
    sleep 1
done

# Start crucible-demo
WORKERS=${WORKERS:-2}
echo "Starting demo with ${WORKERS} worker(s)..."
/h2o-bench-tpcc/build/h2o-bench-tpcc -a "$WORKERS" -c /etc/foundationdb/fdb.cluster -p "$PORT" &
DEMO_PID=$!
sleep 2

# Benchmark
echo ""
echo "=== Benchmark ==="
wrk -t2 -c16 -d10s http://127.0.0.1:${PORT:-8080}/ping 2>&1
echo ""

wait $DEMO_PID