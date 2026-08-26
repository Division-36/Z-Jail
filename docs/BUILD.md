# Build

## Requirements

- GCC 11+ (tested on 11.4, 13.2, 15.2)
- Linux kernel >= 5.4 (namespaces, seccomp-BPF, pivot_root)
- No external libraries — just the standard C toolchain

## Targets

```
make              # build z_jail (~81 KiB unstripped, static by default)
make clean        # remove build artifacts
make check        # smoke test (runs --version)
make test         # run full test suite (18 scenarios, needs root)
make install      # install to $DESTDIR/usr/local/bin + man page
make uninstall    # remove installed files
make dist         # create release tarball in _dist/
make build_id     # print the build ID string
make help         # show all targets
make valgrind-check  # run under Valgrind (leak check)
make pre-commit-check  # check for tab indentation issues
```

## Compile-time Options

```sh
make STATIC=0              # PIE (ASLR-hardened) build instead of static
make CC=clang              # use a different compiler
make CFLAGS="-O3 -march=native"  # custom flags
```

## Test

```sh
make -C tests setup          # build test payloads and root filesystems
sudo bash tests/run_tests.sh  # 18 scenarios (needs root for namespaces)
```

Or simply:

```sh
make test   # does both setup and run_tests.sh
```
