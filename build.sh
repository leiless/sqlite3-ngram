#!/bin/bash

set -euf -o pipefail
#set -x

cd "$(dirname "$0")"

if [ ! -L src/sqlite ]; then
    ./download-sqlite.sh
fi

mkdir -p build
pushd build > /dev/null
    cmake ..
    make clean all
popd > /dev/null

