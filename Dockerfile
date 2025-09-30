FROM ubuntu:24.04
RUN apt update -y && apt-get upgrade -y
RUN apt install -y ninja-build cmake libacl1-dev clang libc++-dev lld python-is-python3
WORKDIR src
COPY . ./
WORKDIR build
RUN  CXX=clang++ CC=clang cmake -Wno-dev -DARCHIVARIUS_STATIC_BUILD=ON -GNinja ..
RUN  cmake --build . --target archivarius
RUN  strip archivarius
