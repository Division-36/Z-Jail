# Threat Model

## In scope
- Arbitrary native code execution by untrusted payload
- Escape via chroot, mount, ptrace, socket, process_vm_writev
- Fork bombs, CPU exhaustion (RLIMIT), memory exhaustion (RLIMIT_AS)
- File descriptor leaks across execve
- setuid / dynamic linker / LD_PRELOAD escalation
- seccomp filter removal or capset re-enablement

## Out of scope
- Kernel zero-days outside the syscall surface
- Hardware side channels (Spectre, Meltdown)
- Co-located container breakout via shared /proc, /sys mounts
- Network egress beyond what `CLONE_NEWNET + no socket` provides
- Resource starvation of sibling sandboxes (requires kernel cgroup support)

## Assumptions
- Host kernel is unmodified Linux ≥ 5.4
- `clone(CLONE_NEWNS|CLONE_NEWPID|...)` succeeds (CAP_SYS_ADMIN
  in initial namespace, or user namespace support)
- Binary is statically linked (mprotect banned; no dynamic loader)
- `--self-hash=<hex>` is configured in production deployments
