Archivarius
===========

An incremental backup tool with versioning.

It's fast:

- To restore the latest saved version, it does not have to first restore the oldest one, and then incrementally refine it. It goes right to the latest, despite being incremental. So depth of your archive does not affect restoration performance.
- Uses modern encryption (ChaCha20+Poly1305) and compression (zstd).
- Unlike [some other incremental backup tools][1], it does not require a few gigs of local cache to work efficiently over network. Neither does it require periodic full-backups. So it's much easier on the network bandwidth.

[1]: http://duplicity.nongnu.org/ "duplicity"

It's reliable:

- It never overwrites files. It only adds new ones, and deletes the obsolete ones. So even in case of power outage while archiving, your archive is safe.

It's simple (less then 5k lines of C++ code) and easy to use.

## Supported attributes

- Modification time (with nanosecond accuracy)
- Symlinks
- ACLs
- Unix premissions

## Getting it

### Pre built binary

You can get a pre-built static binary for x86_64 and arm64 linux, with no side dependencies (e.g. distro agnostic), at the releases section.

### Building via docker

The easiest way to build. Just run the `docker-build.sh` and get the binary at `docker-build-out/`
The binary has no dependency on any library and can be used with any linux distribution.

### Building the classic way

Or you can build it yourself from sources. The only system dependency is libacl and a C++23 compiler. Clang with libc++ produces the most efficient code for the tool. To build, make sure you have clang, libc++ and lld installed on your system. For ubuntu the packages are `clang libc++-dev libc++abi-dev lld`.
You will also need ninja or make, and cmake.

Now you are ready to configure the build

    git clone https://github.com/dsvi/archivarius.git
    cd archivarius
    mkdir build
    cd build

If you prefer a static build, with all the dependencies built into, use the `ARCHIVARIUS_STATIC_BUILD=ON` option. This way the resulting executable depends only on the linux kernel.

    CXX=clang++ CC=clang cmake -DARCHIVARIUS_STATIC_BUILD=ON ..

Otherwise:

    CXX=clang++ CC=clang cmake ..

Then run the build:

    cmake --build . --target archivarius

## Using it

Create [archivarius.conf](./docs/config%20file%20format.md), and run the tool:

    archivarius archive

The command `archive` tells archivarius to execute all the tasks in the config. You might want to add it to cron, to do it daily.
To restore an archive run:

    archivarius restore archive=path/to/archive target-dir=/path/to/restore/archive/to/

## License

The software is provided under [Zlib licence](https://en.wikipedia.org/wiki/Zlib_License):

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
