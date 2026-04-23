# Z-Jail

Pure-C sandbox engine. Zero external dependencies.
Hand-rolled BPF, BLAKE2b, and mount-namespace isolation.

**Build ID:** `Z-Jail/v1+dev`
**Audit schema:** `z-jail.audit/v1`

## Quick start
```sh
make
./z_jail --root=. --seccomp-enforce -- /bin/true
```

## Architecture

7 anti-escape layers applied in strict order inside `child_run()`:

1. **Truthimatics** — evidence-weighted verdict engine
2. **Namespaces** — CLONE_NEWNS|NEWPID|NEWNET|NEWIPC|NEWUTS
3. **pivot_root** — atomic bind-mount + pivot_root, umount old root
4. **Capabilities** — capset(all-zero) + securebits locked
5. **NO_NEW_PRIVS** — prevent setuid / LD_PRELOAD escalation
6. **seccomp-BPF** — whitelist-v1: 15 syscalls + 2 mmarg restrictions
7. **Audit** — JSON audit trail, content fingerprinting

## Security
- mprotect is **banned** — payloads must be statically linked
- mmap PROT_EXEC **banned** — W^X enforced via arg-restricted BPF
- chroot, mount, ptrace, socket, clone, fork — all **blocked**
- `--self-hash=<hex>` for binary integrity verification

## Build
```sh
make          # → z_jail (~130 KiB, PIE, full RELRO)
make clean
make build_id
```

## Test
```sh
make -C tests setup
bash tests/run_tests.sh
```

## Requirements
- GCC ≥ 11 (tested 15.2.0)
- Linux kernel ≥ 5.4
- No external libraries — pure C99 + glibc
