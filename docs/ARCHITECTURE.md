# Architecture
7 defence-in-depth layers: rlimits, fd scrub, PR_SET_DUMPABLE,
pivot_root, NO_NEW_PRIVS, capability drop, seccomp-BPF whitelist.
Audit trail written after child exit.
Data flow: CLI -> parse_args -> clone(namespaces) -> child_run -> execve -> audit

The audit layer writes tamper-evident JSON: canonical BLAKE2b-256 content
fingerprint, `prev_hash` chaining, append-only (`chattr +a`) storage, and
whole-path symlink rejection (`openat2(RESOLVE_NO_SYMLINKS)`).
See `docs/AUDIT_SCHEMA.md`.
