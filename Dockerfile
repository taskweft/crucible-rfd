FROM foundationdb/foundationdb:7.3.79 AS fdb

# Compile stage: use Rocky Linux 9 (same glibc as FDB runtime)
FROM rockylinux:9-minimal AS compile

RUN microdnf install -y cmake gcc gcc-c++ make openssl-devel zlib-devel \
      pkgconfig diffutils curl binutils tar gzip perl && \
    microdnf clean all

# FDB runtime lib from official image (multi-arch)
COPY --from=fdb /usr/lib/libfdb_c.so /usr/lib/

# FDB headers — arch-independent, extract from amd64 deb
ARG FDB_VERSION=7.3.79
RUN curl -sL "https://github.com/apple/foundationdb/releases/download/${FDB_VERSION}/foundationdb-clients_${FDB_VERSION}-1_amd64.deb" -o /tmp/fdb.deb && \
    cd /tmp && ar x fdb.deb && tar xf data.tar.gz && \
    cp -r usr/include/foundationdb/ /usr/include/ && \
    rm -rf /tmp/fdb.deb /tmp/control.tar.gz /tmp/data.tar.gz /tmp/debian-binary usr/
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
RUN curl -sL https://github.com/wg/wrk/archive/refs/tags/4.2.0.tar.gz -o /tmp/wrk.tar.gz && \
    cd /tmp && tar xzf wrk.tar.gz && cd wrk-4.2.0 && \
    make -j$(nproc) && cp wrk /usr/local/bin/

# Runtime stage — use FDB image (has fdbmonitor + libfdb_c + multi-arch)
FROM foundationdb/foundationdb:7.3.79

COPY --from=compile /tmp/build/build/crucible-demo /usr/local/bin/crucible-demo
COPY --from=compile /usr/local/bin/wrk /usr/local/bin/wrk
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh && ldconfig

ENV LD_LIBRARY_PATH=/usr/lib

EXPOSE 8080
ENTRYPOINT ["/entrypoint.sh"]
