Archivarius
===========

An incremental backup tool with versioning.

It's fast:

- To restore the latest saved version, it does not have to first restore the oldest one, and then incrementally refine it. It goes right to the latest, despite being incremental. So depth of your archive does not affect restoration performance.
- Uses modern encryption (ChaCha20+Poly1305) and compression (zstd).
- Unlike [some other incremental backup tools][1], it does not require a few gigs of local cache to work efficiently over network. Neither does it require periodic full-backups. So it's much easier on network bandwidth.

[1]: http://duplicity.nongnu.org/ "duplicity"

It's reliable:

- Never overwrites its files. It only adds new ones, and deletes obsolete ones. So even in case of power outage while archiving, your archive is safe.

It's simple (less then 10k lines of C++ code) and easy to use.

## Supported attributes

- Modification time (with nanosecond accuracy)
- Symlinks
- ACLs
- Unix premissions

## Getting it

You can get pre-built static binary for linux with no side dependencies [here.](https://github.com/dsvi/archivarius/releases)

Or you can build it yourself from sources. The only dependency is libacl and libzstd (`libacl1-dev libzstd1-dev` package for Ubuntu) and a C++20 conformant compiler:

    git clone https://github.com/dsvi/archivarius.git
    cd archivarius
    git checkout latest
    mkdir build
    cd build

Clang with libc++ produces more efficient code. To build, make sure you have clang, libc++ and lld installed on your system. For ubuntu 19.10 the packages are `clang-9 libc++-9-dev libc++abi-9-dev lld-9`.
You will also need make and cmake 3.15 of newer. You can get it from https://github.com/Kitware/CMake/releases/download/. To install, simply untar into /usr/local, like so:

    wget https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0-Linux-x86_64.tar.gz
    tar zxf cmake-3.17.0-Linux-x86_64.tar.gz
    rsync -au ./cmake-3.17.0-Linux-x86_64/ /usr/local/

Now you are ready to configure the build

    CXX=clang++-9 CC=clang-9 cmake ..

If you prefer static build, with all the dependencies built into, use the `ARCHIVARIUS_STATIC_BUILD=ON` option. This way the resulting executable depends only on the linux kernel.

    CXX=clang++-9 CC=clang-9 cmake -DARCHIVARIUS_STATIC_BUILD=ON ..

Then run the build:

    cmake --build . --target archivarius

## Using it

Create [archivarius.conf](./docs/config%20file%20format.md), and run the tool:

    archivarius archive

The command `archive` tells archivarius to execute all the tasks in the config. You might want to add it to cron, to do it daily.
To restore an archive run:

    archivarius restore archive=path/to/archive target-dir=/path/to/restore/archive/to/

## Licence 

The software is provided under [Zlib licence](https://en.wikipedia.org/wiki/Zlib_License):

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
