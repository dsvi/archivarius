## Building the classic way

Or you can build it yourself from sources. The only system dependency is `libacl` and a C++23 compiler. Clang with libc++ produces the most efficient code for the tool. To build, make sure you have clang, libc++ and lld installed on your system. You will also need ninja or make, and cmake.

Now you are ready to configure the build

```
git clone --depth=1 https://github.com/dsvi/archivarius.git
cd archivarius
mkdir build
cd build
```

If you prefer a static build, with all the dependencies built into, use the `ARCHIVARIUS_STATIC_BUILD=ON` option. This way the resulting executable depends only on the linux kernel.

```
CXX=clang++ CC=clang cmake -DARCHIVARIUS_STATIC_BUILD=ON ..
```

Otherwise:

```
CXX=clang++ CC=clang cmake ..
```

Then run the build:

```
cmake --build . --target archivarius
```
