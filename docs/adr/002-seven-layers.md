# ADR 2: Seven Ordered Isolation Layers
## Context
Single sandbox mechanisms can be bypassed.
## Decision
Stack 7 ordered kernel isolation mechanisms: resource limits, fd scrubbing,
PR_SET_DUMPABLE, pivot_root, NO_NEW_PRIVS, capability drop, seccomp-BPF.
## Consequences
+ Defence-in-depth: bypassing one layer doesn't escape the sandbox
+ Irreversible monotonic privilege reduction
- Increased latency (~8ms) from multiple syscalls
