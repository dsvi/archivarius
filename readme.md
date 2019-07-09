Archivarius
===========

An incremental backup tool with versioning.

It's fast:

- To restore the latest saved version, it does not have to first restore the oldest one, and then incrementally refine it. It goes right to the latest, despite being incremental. So depth of your archive does not affect restoration performance.
- Uses modern encryption (ChaCha20+Poly1305) and compression (zstd).
- Unlike [some other incremental backup tools][1], it does not require a few gigs of local cache to work efficiently over network.

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
	ccmake .. 
	make

After that, create [archivarius.conf](./docs/config file format.md) file at one of those paths:

	~/.config
	/usr/local/etc
	/etc

And launch the tool `archivarius archive`
