#!/bin/bash

set -euf -o pipefail
#set -x

cd "$(dirname "$0")"

if [ ! -L src/sqlite ]; then
    ./download-sqlite.sh
fi

cmake -S . -B build
cmake --build build --target clean
cmake --build build

