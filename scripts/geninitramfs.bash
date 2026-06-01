#!/bin/bash -x

set -e
pwd=$(pwd)
cd ${GOPATH}/src/github.com/u-root/u-root && u-root \
    -defaultsh $(which bash) \
    -uinitcmd "/bbin/gosh" \
    -o ${pwd}/initramfs \
    -files $(which bash) \
    -files $(which lsblk) \
    -files $(which lspci) \
    -files ${pwd}/scripts/uinit.sh:/uinit.sh \
    core boot
