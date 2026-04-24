# Architecture
7 defence-in-depth layers: Truthimatics, Namespaces, pivot_root,
Capability drop, NO_NEW_PRIVS, seccomp-BPF whitelist-v1, Audit.
Data flow: CLI -> parse_args -> clone(namespaces) -> child_run -> execve -> audit
