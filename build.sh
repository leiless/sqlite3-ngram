#!/bin/bash

set -euf -o pipefail
#set -x

cd "$(dirname "$0")"

if [ ! -L src/sqlite ]; then
    ./download-sqlite.sh
fi

pushd src/proto
protoc --cpp_out=. highlight_result.proto
popd

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target clean
cmake --build build

