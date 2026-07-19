# Roadmap

## v1 (current)
- Seven ordered isolation layers
- BLAKE2b-256 content fingerprinting
- Audit JSON output (`z-jail.audit/v1`)
- 18 test scenarios (indexed 0\u201317)
- man page, shell completions (bash, zsh, fish)
- CI: build, coverage, lint, fuzz, weekly

## v2 (planned)
- seccomp policy configuration via external JSON file
- Custom namespace flags per sandbox instance (`--CLONE_NEWNS`, `--no-net`, etc.)
- Configurable syscall whitelist entries at runtime
- Performance profiling hooks for CI latency tracking
- Release signing (minisign)

## v3 (stretch)
- User namespace support (rootless operation)
- Landlock LSM integration
- seccomp notify (SECCOMP_IOCTL_NOTIF_RECV) for dynamic policy
- BPF CO-RE for cross-kernel seccomp portability
