# seccomp-BPF whitelist-v1

24 syscalls allowed; everything else is killed (`SECCOMP_RET_KILL`):

```
read, write, openat, close, lseek, brk, mmap(arg-restricted), munmap,
execve, exit_group, rt_sigaction, rt_sigprocmask, getrandom,
clock_gettime, fstat, arch_prctl, mprotect(arg-restricted), prlimit64(arg-restricted),
readlinkat, rseq, set_robust_list, set_tid_address, access, pread64
```

The last nine (`arch_prctl` .. `pread64`) are the C-runtime startup calls that
a modern statically-linked glibc program needs before `main`; without them
even a trivial static binary is killed at load time.

Argument-level restrictions:

```
mmap     : flags == 0x22 (MAP_PRIVATE | MAP_ANONYMOUS)  AND  (prot & PROT_EXEC) == 0
mprotect : (prot & PROT_EXEC) == 0        # preserves W^X; no page becomes executable
prlimit64: new_limit == NULL              # read-only; the guest cannot raise its own rlimits
```

The `mprotect` and `prlimit64` restrictions are essential: an unrestricted
`mprotect` would let a guest turn a writable page executable (defeating the
`mmap` PROT_EXEC ban), and an unrestricted `prlimit64` would let it lift the
CPU/AS/NPROC limits set by the sandbox.

Because the policy forbids executable file mappings, **only statically-linked
targets run**: a dynamically-linked program is rejected because its loader
(`ld.so`) must `mmap` shared-library code with `PROT_EXEC`.

The filter checks the architecture (`AUDIT_ARCH_X86_64`) first and rejects
mismatches. For each whitelisted syscall number a comparison is emitted
that either allows the call or falls through to the terminating action.

See `docs/BENCHMARKS.md` for measured performance and
`docs/AUDIT_SCHEMA.md` for the audit record format.
