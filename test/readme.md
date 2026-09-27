# The self test

The test is compiled into a separate target, `archivarius-test`, which is not built by default:

    cmake --build build --target archivarius-test
    ./build/archivarius-test

The test code is `src/testing.c++`. The test finds `test/test.conf` on its own (see Configs),
so it can be started from any dir.

There are three parts. `filters_test` and `multi_version_test` are fast, `full_test` is the long
one (it restores a few gigabytes for every version it made).

# Where the test works

All the work happens under

    /tmp/archivarius-test

which is a tmpfs on most systems, so the test runs mostly in ram. The dir is wiped out at the
start of the long test. There:

    tmp/                 the working copy of the seed. Modified, step by step
    arc/                 the archive, made out of tmp/
    add/                 a copy of the seed 'add'. One file from it is copied into tmp/ per step
    rmv                  a copy of the seed 'rmv' (see below)
    filters/<scenario>/  the small tree and the archive of one filter scenario

# The seed

The long test works on a real tree of files (a few gigs), read from

    ~/temp/atest/
            ├── src/
            ├── add/
            └── rmv

Set `ARCHIVARIUS_TEST_SEED` to keep it somewhere else. The seed is only read, never modified.

    src/   the initial content of tmp/
    add/   from this dir files in the working dir will be updated gradually, one per test step
    rmv    a list of paths, relative to the working dir. One of them is removed on every step

It's a good idea to have a dir named `ignore` in `src/`. Both configs exclude `ignore`, so it
must not get into the archive, and must not appear in a restored tree.

# How the long test works

1. The seed `src/` is copied into `tmp/`. `add/` and `rmv` are copied as they are.
2. Then, as long as there is something to add or to remove:
   - the state of `tmp/` is remembered as the expected state of this step;
   - archivarius archives `tmp/` with `test/test.conf`;
   - the next file from `add/` is copied into `tmp/`, and the next path from `rmv` is removed
     from it.
   So there is one archive version per step, and every version has its expected state.
3. Then every version is restored into `tmp/`, newest first, and compared with the expected
   state of its step: the set of paths, their types, sizes, modification times (with
   nanosecond accuracy), unix permissions and ACLs. Symlinks are compared by their targets.
4. The GC test: the same tree is archived with `test/test-1s.conf`, which has
   `max-storage-time 1s`, so it has to leave exactly one version in the archive.
5. The lock test: opening the same archive a second time while it is already open has to fail.
6. The remaining version is restored once more and compared with the final state.

# Configs

`test/test.conf` and `test/test-1s.conf` are the configs of the long test. The paths in them
are absolute, and point into `/tmp/archivarius-test`, where the test works. The only difference
between them is `max-storage-time`: `6m` (nothing is removed) and `1s` (for the GC test).

They have no `password` and no `compression`, so that the long test runs fast. The encryption and
the compression are covered by the filters scenarios, which do use a password. So the long test
archives have to be opened without a password (`long_test_password` in `src/testing.c++`).

The filter scenarios write their own configs into their own scratch dirs.

The test looks for these files in `$ARCHIVARIUS_TEST_DIR`, then in `./test`, then up the dir
tree from the location of the test executable.

# The filters test

`filters_test` checks the `root` `include` and `exclude` logic on a small tree. For every
scenario the tree is created, the config is written, archivarius archives it, then:

- the paths the archive version holds are compared with the expected ones;
- the version is restored, and the restored tree is compared with the expectation, contents
  and symlink targets included.

A failing scenario does not stop the others. All of them are reported at the end.

The sample tree:

    a.txt                   'a'
    empty.txt               an empty file
    big.bin
    excl.txt                to be excluded
    skip/ignored.txt
    dir1/f1.txt
    dir1/skip/ignored.txt
    dir1/nested/f2.txt
    dir2/deep/f3.txt
    link                    a symlink to a.txt
    dirlink                 a symlink to dir1
    excl_link               a symlink to big.bin. to be excluded
    badlink                 a dangling symlink

What every scenario checks:

    root-only                   everything from root, except the excluded. A dir which does not
                                exist is excluded without any complaint
    include-over-exclude        'include' has priority over 'exclude': dir1 is out, but
                                dir1/nested is in
    include-over-exclude-deep   the same, through several excluded dirs
    include-list                only the included. root is still what the stored paths are
                                relative to
    single-file                 a single file, with no dirs in the archive at all. The
                                target dir has to be created by the restore
    trailing-slashes            a trailing '/' in a config path has to mean the same as without it
    all-excluded                nothing is left to archive, so nothing is stored
    dot-not-allowed             '.' is not allowed in a path, so this config has to be refused
    dotdot-not-allowed          the same for '..'

`multi_version_test` archives the same tree twice with different filters, after changing the
tree in between, and checks that restoring either version gives the tree as it was for that
version. It also runs `archivarius test` on the archive, which checks the references and the
contents of all the versions.

Note that a directory which is not in the archive still has to appear in a restored tree,
because a file from it is there. This is how 'include' overriding 'exclude' works.
