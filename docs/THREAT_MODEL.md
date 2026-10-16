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
- Kernel zero-days, including via the allowed 24-call surface, and host/hypervisor bugs
- Hardware side channels (Spectre, Meltdown)
- Co-located container breakout via shared /proc, /sys mounts (the sandbox mounts no /proc or /sys of its own; shared-host mounts are out of scope either way)
- Network egress beyond what `CLONE_NEWNET + no socket` provides; harness/host-operated egress on the guest's behalf (registry/cache proxy, metadata endpoints, model-API server-side fetch) is outside guest seccomp coverage entirely
- Resource starvation of sibling sandboxes (requires kernel cgroup support)
- Audit-trail tampering by the host superuser (root); see above
- Rootfs staging symlink/filename races and check-then-execute TOCTOU without descriptor-pinned execution
- Covert channels (timing, exit codes, resource usage)

## Assumptions
- Host kernel is unmodified Linux ≥ 5.4, x86_64 only; other arches/kernels unmeasured
- `clone(CLONE_NEWNS|CLONE_NEWPID|...)` succeeds (requires CAP_SYS_ADMIN
  in the initial namespace; no user-namespace/rootless path yet)
- Binary is statically linked (mprotect restricted to non-EXEC; no dynamic loader)
- `--self-hash=<hex>` is configured in production deployments
- Defaults are insecure: integrity pinning is OFF unless `--self-hash` is given, `--no-audit` is permitted, and there is no fail-closed production profile; production MUST set `--self-hash` and deny `--no-audit`
- Sandbox manager (privileged parent, audit emitter) is fully trusted; its compromise voids all guest and audit guarantees; no manager sandboxing exists
