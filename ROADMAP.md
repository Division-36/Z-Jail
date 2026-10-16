# Roadmap

## v1.1.0 (current)
- Seven ordered isolation layers
- BLAKE2b-256 content fingerprinting
- Audit JSON output (`z-jail.audit/v1`) with `append_only` tracking and `fdatasync` durability
- seccomp-BPF jump offset validation (fail closed on overflow)
- `--no-audit` flag to disable audit file creation
- `STATIC=1` build option for self-contained binary
- 18 test scenarios (indexed 0\u201317) + 8 seccomp unit checks
- man page, shell completions (bash, zsh, fish)
- seccomp robustness benchmark (`_benchmarks/seccomp_robustness/`)
- CI: build, coverage, lint, weekly

## v2 (planned)
- seccomp policy configuration via external JSON file
- Custom namespace flags per sandbox instance (`--CLONE_NEWNS`, `--no-net`, etc.)
- Configurable syscall whitelist entries at runtime
- Performance profiling hooks for CI latency tracking
- Release signing (minisign)
- Fuzz integration (oss-fuzz / libFuzzer)

## v3 (stretch)
- User namespace support (rootless operation)
- Landlock LSM integration
- seccomp notify (SECCOMP_IOCTL_NOTIF_RECV) for dynamic policy
- BPF CO-RE for cross-kernel seccomp portability

## Paper controls backlog (CCGrid submission; unplanned unless adopted)
- F1 harness-side default-deny egress + dependency scoping (operator guidance)
- F2 minimal-runtime routing guidance for capable agents (operator guidance)
- F4 control-group (cgroup) v2 per-instance caps + kill-on-budget + accounting
- F5 off-host audit tip commitment (blocking commit, loss window, fail-closed forwarder, `--no-audit` forbidden; key custody off-host)
- F6b descriptor-pinned execution + fail-closed expected-digest production profile + manager minimization
- F7 evaluation hygiene (refusal scoping, trajectory monitoring, forensic models, attribution channel)
