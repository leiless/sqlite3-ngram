FROM ubuntu

ARG DEBIAN_FRONTEND=noninteractive

# https://stackoverflow.com/questions/24648357/compiling-a-static-executable-with-cmake
RUN set -eux && \
    apt-get update && \
    TMP_PKG='build-essential' && \
    apt-get install -y --no-install-recommends ca-certificates git curl unzip cmake g++ make $TMP_PKG && \
    git clone --depth=1 https://github.com/google/glog.git && \
    cd glog && \
    cmake -S . -B build -DBUILD_SHARED_LIBS=OFF && \
    cmake --build build --target install && \
    mv build/lib/cmake/glog/glog-modules.cmake /usr/local/lib/cmake/glog && \
    cd .. && rm -rf glog && \
    URL=https://github.com/protocolbuffers/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz && \
    curl -L "$URL" -o "$(basename "$URL")" && \
    tar xf protobuf-2.6.1.tar.gz --no-same-owner && \
    cd protobuf-2.6.1 && \
    ./configure && make -j$(nproc) && make install && \
    cd .. && rm -rf protobuf-2.6.1 && \
    apt-get autoremove -y $TMP_PKG && \
    rm -rf /var/lib/apt/lists/* && \
    protoc --version && \
    g++ --version

