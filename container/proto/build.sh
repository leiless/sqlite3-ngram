#!/bin/bash

set -eufo pipefail
#set -x

cd "$(dirname "$0")"

TAG='local/ubuntu-protoc2'
podman build -t $TAG .

ROOT=$(dirname "$(dirname "$PWD")")
podman run -it --rm \
    -v "$ROOT/src/proto":/proto \
    $TAG bash

