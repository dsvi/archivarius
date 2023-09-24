FROM debian:sid
RUN apt-get update -y && apt-get upgrade -y
RUN apt-get install -y ninja-build cmake libacl1-dev clang libc++-dev libc++abi-dev lld python-is-python3
WORKDIR src
COPY . ./
WORKDIR build
RUN  CXX=clang++ CC=clang cmake -Wno-dev -DARCHIVARIUS_STATIC_BUILD=ON -GNinja ..
RUN  cmake --build . --target archivarius
#RUN  strip archivarius


