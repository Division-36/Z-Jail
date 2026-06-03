# ADR 1: No External Dependencies
## Context
libseccomp, libcap, libcrypto are common for sandboxing.
## Decision
Hand-roll BPF, capabilities, BLAKE2b to minimize supply chain risk and binary size.
## Consequences
+ No linking issues, small binary (~130 KiB)
- More code to maintain and audit
