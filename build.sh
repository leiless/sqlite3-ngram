#!/bin/bash

set -euf -o pipefail
#set -x

cd "$(dirname "$0")"

if [ ! -L src/sqlite ]; then
    echo "ERR: Please run download-sqlite.sh first!"
    exit 1
fi

mkdir -p build
pushd build > /dev/null
    cmake ..
    make clean all
popd > /dev/null

