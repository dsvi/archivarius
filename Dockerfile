FROM ubuntu:20.04
RUN apt-get update -y && apt-get upgrade -y
RUN apt-get install -y ninja-build cmake libacl1-dev clang libc++-dev libc++abi-dev lld libzstd-dev
WORKDIR build
COPY . ./
WORKDIR build
RUN  CXX=clang++ CC=clang cmake -DARCHIVARIUS_STATIC_BUILD=ON -GNinja ..
RUN  cmake --build . --target archivarius
RUN  strip archivarius

