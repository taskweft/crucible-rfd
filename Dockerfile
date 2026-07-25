FROM foundationdb/foundationdb:7.3.79 AS fdb

FROM buildpack-deps:26.04 AS compile

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq && apt-get install --no-install-recommends -qqy \
      cmake gcc libssl-dev libz-dev make pkg-config curl

# FDB runtime lib from official image (multi-arch)
COPY --from=fdb /usr/lib/libfdb_c.so /usr/lib/

# FDB headers — arch-independent, extract from amd64 deb
ARG FDB_VERSION=7.3.79
RUN curl -sL "https://github.com/apple/foundationdb/releases/download/${FDB_VERSION}/foundationdb-clients_${FDB_VERSION}-1_amd64.deb" -o /tmp/fdb.deb && \
    mkdir -p /tmp/fdb-extract && cd /tmp/fdb-extract && \
    dpkg-deb -x /tmp/fdb.deb . && \
    cp -r usr/include/foundationdb/ /usr/include/ && \
    rm -rf /tmp/fdb.deb /tmp/fdb-extract
RUN ldconfig

# Build libh2o from vendored source
COPY vendor/h2o /tmp/h2o
RUN cmake -B /tmp/h2o/build -DCMAKE_BUILD_TYPE=Release \
      -DWITH_MRUBY=off -DWITH_UV=off -S /tmp/h2o && \
    cmake --build /tmp/h2o/build -j$(nproc) && cmake --install /tmp/h2o/build

# Build crucible-demo
COPY h2o-bench-tpcc/ /tmp/build
RUN cmake -B /tmp/build/build -DCMAKE_BUILD_TYPE=Release -S /tmp/build && \
    cmake --build /tmp/build/build -j$(nproc)

# Build wrk
RUN git clone --depth=1 https://github.com/wg/wrk.git /tmp/wrk && \
    cd /tmp/wrk && make -j$(nproc) && cp wrk /usr/local/bin/

# Runtime stage — use FDB image (has fdbmonitor + libfdb_c + multi-arch)
FROM foundationdb/foundationdb:7.3.79

COPY --from=compile /tmp/build/build/crucible-demo /usr/local/bin/crucible-demo
COPY --from=compile /usr/local/bin/wrk /usr/local/bin/wrk
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENV WORKERS=2
ENV PORT=8080

EXPOSE 8080
ENTRYPOINT ["/entrypoint.sh"]
