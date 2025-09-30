#!/usr/bin/bash

renice -n 19 $$
cd ..
mkdir release

set -e
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
for platform in amd64 riscv64 arm64
#for platform in amd64 arm64
do
    ./docker-build.sh "--platform linux/$platform"
    export ZSTD_CLEVEL=19
    tar -caf release/archivarius-$platform.tar.zst -C docker-build-out archivarius
done

rm -rf docker-build-out

docker container prune --force
docker image  prune --all --force
