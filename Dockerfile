FROM alpine:3.21

RUN apk add --no-cache \
        acl-dev \
        acl-static \
        attr-dev \
        binutils \
        clang \
        cmake \
        coreutils \
        diffutils \
        git \
        lld \
        libc++-dev \
        libc++-static \
        linux-headers \
        llvm \
        make \
        ninja \
        p7zip \
        procps \
        python3

WORKDIR /src
COPY . ./
WORKDIR build
RUN  CXX=clang++ CC=clang cmake -Wno-dev -DARCHIVARIUS_STATIC_BUILD=ON -GNinja ..
RUN  cmake --build . --target archivarius
RUN  strip archivarius
