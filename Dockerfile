FROM buildpack-deps:26.04 AS compile

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq && apt-get install --no-install-recommends -qqy \
      cmake gcc libssl-dev libz-dev make pkg-config

# Build libh2o from vendored source
COPY vendor/h2o /tmp/h2o
RUN cmake -B /tmp/h2o/build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="-flto=auto" \
      -DWITH_MRUBY=off -DWITH_UV=off -S /tmp/h2o && \
    cmake --build /tmp/h2o/build -j$(nproc) && cmake --install /tmp/h2o/build

# Install FDB client for compilation
ARG FDB_VERSION=7.3.79
RUN curl -LSs "https://github.com/apple/foundationdb/releases/download/${FDB_VERSION}/foundationdb-clients_${FDB_VERSION}-1_amd64.deb" -o /tmp/fdb.deb && \
    dpkg -i /tmp/fdb.deb && rm /tmp/fdb.deb

# Build crucible-demo
WORKDIR /tmp/build
COPY h2o-bench-tpcc/ .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -S . && \
    cmake --build build -j$(nproc)

# Install wrk
RUN git clone --depth=1 https://github.com/wg/wrk.git /tmp/wrk && \
    cd /tmp/wrk && make -j && cp wrk /usr/local/bin/

# Runtime stage
FROM ubuntu:26.04

ARG FDB_VERSION=7.3.79
RUN apt-get update -qq && \
    apt-get install --no-install-recommends -qqy libssl3 curl ca-certificates adduser && \
    curl -LSs "https://github.com/apple/foundationdb/releases/download/${FDB_VERSION}/foundationdb-clients_${FDB_VERSION}-1_amd64.deb" -o /tmp/fdb-client.deb && \
    dpkg -i /tmp/fdb-client.deb && \
    curl -LSs "https://github.com/apple/foundationdb/releases/download/${FDB_VERSION}/foundationdb-server_${FDB_VERSION}-1_amd64.deb" -o /tmp/fdb-server.deb && \
    dpkg -i /tmp/fdb-server.deb && apt-get install -f -qqy && \
    rm /tmp/fdb-*.deb && rm -rf /var/lib/apt/lists/*

COPY --from=compile /tmp/build/build/crucible-demo /usr/local/bin/crucible-demo
COPY --from=compile /usr/local/bin/wrk /usr/local/bin/wrk

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENV LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
ENV WORKERS=2
ENV PORT=8080

EXPOSE 8080
ENTRYPOINT ["/entrypoint.sh"]
