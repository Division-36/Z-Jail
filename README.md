# Z-Jail

Multi-layer sandbox for native code execution on Linux.

**7 defence-in-depth layers:** Truthimatics → namespaces → pivot_root →
capabilities → NO_NEW_PRIVS → seccomp-BPF → audit.

Zero external dependencies. ~130 KiB PIE binary. BLAKE2b content hashing.

## Quick Start

```
make && sudo ./z_jail --root=./roots --seccomp-enforce -- /bin/sh
```

## Build

```
make          # z_jail
make install  # install to /usr/local
make dist     # release tarball
make -C tests setup && bash tests/run_tests.sh  # run tests (17 scenarios)
```

## Requirements

- Linux 5.4+ (kernel namespaces, seccomp-BPF, pivot_root)
- GCC 11+ (tested on 11.4, 13.2, 15.2)
- No external libraries needed

## Architecture

```
CLI → parse_args → clone(CLONE_NEWNS|NEWPID|NEWNET|NEWIPC|NEWUTS)
  → child_run:
    1. setrlimit
    2. fd scrub
    3. PR_SET_DUMPABLE=0
    4. pivot_root (bind + pivot + umount)
    5. PR_SET_NO_NEW_PRIVS
    6. capset all-zero + securebits locked
    7. seccomp-BPF whitelist-v1 (15 syscalls)
    8. signal parent via pipe
    9. execve target
  → parent collects exit status + audit JSON
```

## Status

[![build](.github/workflows/build.yml)](.github/workflows/build.yml)
[![coverage](.github/workflows/coverage.yml)](.github/workflows/coverage.yml)

## License

MIT
