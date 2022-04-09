#!/usr/bin/bash

set -e

rm -rf docker-build-out
docker build --tag archivarius-build .
mkdir docker-build-out
id=$(docker create archivarius-build)
docker cp $id:/build/build/archivarius docker-build-out/
docker rm $id
docker image rm archivarius-build

HI='\033[0;32m'
NC='\033[0m' # No Color

echo
echo  =^..^=   =^..^=   =^..^=    =^..^=    =^..^=    =^..^=    =^..^=    =^..^=    =^..^=    =^..^=    =^..^=
echo
echo -e Output is at ${HI}./docker-build-out${NC} folder.
echo -e You might want to use ${HI}docker image  prune --all${NC} to clean out the build leftovers
echo

