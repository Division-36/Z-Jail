# ADR 2: Seven-Layer Defence
## Context
Single sandbox mechanisms can be bypassed.
## Decision
Stack 7 independent layers: Truthimatics, namespaces, pivot_root,
capabilities, NO_NEW_PRIVS, seccomp-BPF, audit.
## Consequences
+ Defence-in-depth: bypassing one layer doesn't escape the sandbox
- Increased latency (~8ms) from multiple syscalls
