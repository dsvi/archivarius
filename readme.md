Archivarius
===========

An incremental backup tool with versioning.

It's fast:

- To restore the latest saved version, it does not have to first restore the oldest one, and then incrementally refine it. It goes right to the latest, despite being incremental. So depth of your archive does not affect restoration performance.
- Uses modern encryption (ChaCha20+Poly1305) and compression (zstd).
- Unlike [some other incremental backup tools][1], it does not require a few gigs of local cache to work efficiently over network. Neither does it require periodic full-backups. So it's much easier on network bandwidth.

It's reliable:

- Never overwrites its files. It only adds new ones, and deletes obsolete ones. So even in case of power outage while archiving, your archive is safe.
- Unlike [some other incremental backup tools][1], restore actually works, without hassle and cryptic errors.

[1]: http://duplicity.nongnu.org/ "duplicity"

It's simple (less then 10k lines of C++ code) and easy to use.

## Supported attributes

- Modification time (with nanosecond accuracy)
- Softlink targets
- ACLs
- Unix premissions

## Getting it

You can get pre-built static binary for linux with no side dependencies [here.](https://bitbucket.org/baltic/archivarius/downloads/archivarius)

Or you can build it yourself:

    hg clone https://bitbucket.org/baltic/archivarius
    cd archivarius
    hg up latest
	mkdir build
	cd build

For Ubuntu you'll need `libacl1-dev` installed.

Now you need a good C++17 conformant compiler. Clang with libc++ produces more effective code for the tool. So do something like this (make sure you have clang, libc++ and lld installed on your system. For ubuntu 18.04 the packages are `clang-8 libc++-8-dev lld-8`):

    CXX=clang++-8 CC=clang-8 cmake ..
    make archivarius

## Using it

Create [archivarius.conf](./docs/config file format.md), and run the tool:

    archivarius archive

The command `archive` tells archivarius to execute all the tasks in the config. You might want to add it to cron, to do it daily.
To restore an archive run:

	archivarius restore archive=path/to/archive target-dir=/home/me/

## Licence 

The software is provided under [Zlib licence](https://en.wikipedia.org/wiki/Zlib_License):

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
