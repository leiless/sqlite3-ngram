#!/bin/bash

set -eufo pipefail
#set -x

cd "$(dirname "$0")"

TAG=local/build-sqlite3-ngram
docker build --rm -t $TAG .

ROOT=$(dirname "$(dirname "$PWD")")
docker run -it --rm \
    --security-opt label=disable --userns keep-id \
    -v "$ROOT":/src \
    $TAG /src/build.sh

