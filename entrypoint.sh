#!/bin/bash
set -euo pipefail

# Start FoundationDB
echo "Starting FoundationDB..."
if [ ! -f /etc/foundationdb/fdb.cluster ]; then
    echo "Docker: single" > /etc/foundationdb/fdb.cluster
fi
/usr/lib/foundationdb/fdbmonitor --conffile /etc/foundationdb/foundationdb.conf \
    --lockfile /var/log/foundationdb/fdbmonitor.pid &

# Wait for FDB to be ready
for i in $(seq 1 30); do
    if fdbcli --exec "status minimal" --timeout 2 2>/dev/null | grep -q "committed"; then
        echo "FDB ready"
        break
    fi
    sleep 1
done

# Start crucible-demo
WORKERS=${WORKERS:-2}
echo "Starting crucible-demo with $WORKERS worker(s)..."
/usr/local/bin/crucible-demo -a "$WORKERS" -c /etc/foundationdb/fdb.cluster -p "$PORT" &
DEMO_PID=$!
sleep 2

# Run benchmark
echo ""
echo "=== Benchmark ==="
echo "Workers: $WORKERS"
echo ""

wrk -t2 -c16 -d10s http://127.0.0.1:${PORT:-8080}/ping 2>&1

echo ""
echo "=== Done ==="

# Keep running if WRK_ONLY is not set
if [ -z "${WRK_ONLY:-}" ]; then
    wait $DEMO_PID
fi
