# History

Z-Jail started as **Axiom-Jail**, a research sandbox for native code execution. Renamed to Z-Jail/v1 after the pivot to a zero-external-dependency design.

Built on WSL2 (GCC 15.2.0, Kali Linux), targeting Linux 5.4+. Development spanned ~6 weeks with 135+ commits.

- **April 2026** — initial prototype with namespaces + pivot_root
- **May 2026** — seccomp-BPF whitelist, BLAKE2b hashing, audit JSON
- **June 2026** — v1 release: 7-layer sandbox, 17 test scenarios, CI/CD
