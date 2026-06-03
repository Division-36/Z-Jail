# ADR 4: Seccomp Whitelist (not blacklist)
## Context
Seccomp-BPF can deny specific syscalls or allow only known-good ones.
## Decision
Whitelist: allow only 15 syscalls needed for static binaries.
Blacklist approach is fragile against new bypass vectors.
## Consequences
+ Strong security guarantee: unknown syscalls are denied by default
- Must update whitelist when applications need new syscalls
