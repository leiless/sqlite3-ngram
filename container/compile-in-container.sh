#!/bin/bash

set -eufo pipefail
#set -x

check_in_container() {
    # https://superuser.com/questions/1021834/what-are-dockerenv-and-dockerinit/1115029#1115029
    # https://github.com/containers/podman/issues/648
    # https://stackoverflow.com/questions/36639062/how-do-i-tell-if-my-container-is-running-inside-a-kubernetes-cluster
    # https://stackoverflow.com/questions/3601515/how-to-check-if-a-variable-is-set-in-bash
    if [ ! -f /.dockerenv ] && [ ! -f /run/.containerenv ] && [ -z ${KUBERNETES_SERVICE_HOST+x} ]; then
        echo "[ERR] This script should be run inside a container environment!"
        exit 1
    fi
}

check_in_container

cd /src
./build.sh

