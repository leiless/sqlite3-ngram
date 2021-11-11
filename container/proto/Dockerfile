FROM ubuntu

RUN set -eux && \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates wget build-essential && \
    mkdir /build-protoc && \
    cd /build-protoc && \
    wget https://github.com/protocolbuffers/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz && \
    tar xf protobuf-2.6.1.tar.gz --no-same-owner && \
    cd protobuf-2.6.1 && \
    ./configure && make -j$(nproc) && make install && \
    cd / && \
    rm -rf /build-protoc && \
    apt-get remove -y wget build-essential && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/* && \
    protoc --version

