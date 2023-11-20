#!/bin/bash -x

set -e
pwd=$(pwd)
cd ${GOPATH}/src/github.com/u-root/u-root && u-root \
    -defaultsh $(which bash) \
    -o ${pwd}/initramfs \
    -files $(which bash) \
    -files $(which lsblk) \
    -files $(which lspci) \
    core boot
