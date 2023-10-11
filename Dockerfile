FROM ubuntu:23.10
RUN apt-get update -y && apt-get upgrade -y
RUN apt install -y wget lsb-release software-properties-common gnupg
RUN bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
RUN apt-get install -y ninja-build cmake libacl1-dev clang-17 libc++-17-dev libc++abi-17-dev lld-17 python-is-python3
WORKDIR src
COPY . ./
WORKDIR build
RUN  CXX=clang++-17 CC=clang-17 cmake -Wno-dev -DARCHIVARIUS_STATIC_BUILD=ON -GNinja ..
RUN  cmake --build . --target archivarius
#RUN  strip archivarius


