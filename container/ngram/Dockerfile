FROM ubuntu

ARG DEBIAN_FRONTEND=noninteractive

# https://stackoverflow.com/questions/24648357/compiling-a-static-executable-with-cmake
RUN set -eux && \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates git curl unzip cmake g++ make && \
    git clone --depth=1 https://github.com/google/glog.git && \
    cd glog && \
    cmake -S . -B build -DBUILD_SHARED_LIBS=OFF && \
    cmake --build build --target install && \
    mv build/lib/cmake/glog/glog-modules.cmake /usr/local/lib/cmake/glog && \
    cd .. && \
    rm -rf glog && \
    rm -rf /var/lib/apt/lists/*

