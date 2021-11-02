FROM ubuntu

ARG DEBIAN_FRONTEND=noninteractive

# https://stackoverflow.com/questions/24648357/compiling-a-static-executable-with-cmake
RUN set -eufx && \
    apt-get update && \
    apt-get install -y git curl unzip cmake g++ && \
    git clone --depth=1 https://github.com/google/glog.git && \
    cd glog && \
    cmake -S . -B build -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF && \
    cmake --build build --target install && \
    cd .. && \
    rm -rf glog && \
    apt-get autoremove -y git && \
    rm -rf /var/lib/apt/lists/*

