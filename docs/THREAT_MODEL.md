# Threat Model

## In scope
- Arbitrary native code execution by untrusted payload
- Escape via chroot, mount, ptrace, socket, process_vm_writev
- Fork bombs, CPU exhaustion (RLIMIT), memory exhaustion (RLIMIT_AS)
- File descriptor leaks across execve
- setuid / dynamic linker / LD_PRELOAD escalation
- seccomp filter removal or capset re-enablement

## Audit trail integrity
Post-execution tampering with the audit record is a distinct attack surface
(the record is written after the child exits). Mitigated with dependency-free
mechanisms (see `docs/AUDIT_SCHEMA.md`):
- In-scope, mitigated: in-place record edits and truncation (append-only
  `chattr +a`); deletion / reordering of interior records (hash chain,
  detectable by an external verifier); symlink redirection of any path
  component (`openat2(RESOLVE_NO_SYMLINKS)`); non-owner writers (DAC 0600).
- Out of scope (inherent): tampering by root / `CAP_LINUX_IMMUTABLE`, which can
  clear the append-only flag and recompute a self-consistent chain. This
  requires an external trust anchor (off-host signing or remote forwarding),
  which is outside the zero-dependency design.

## Out of scope
- Kernel zero-days outside the syscall surface
- Hardware side channels (Spectre, Meltdown)
- Co-located container breakout via shared /proc, /sys mounts
- Network egress beyond what `CLONE_NEWNET + no socket` provides
- Resource starvation of sibling sandboxes (requires kernel cgroup support)
- Audit-trail tampering by the host superuser (root); see above

## Assumptions
- Host kernel is unmodified Linux ≥ 5.4
- `clone(CLONE_NEWNS|CLONE_NEWPID|...)` succeeds (CAP_SYS_ADMIN
  in initial namespace, or user namespace support)
- Binary is statically linked (mprotect banned; no dynamic loader)
- `--self-hash=<hex>` is configured in production deployments
