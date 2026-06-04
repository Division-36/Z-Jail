# Roadmap

## v1 (current)
- 7-layer defence-in-depth sandbox
- BLAKE2b-256 content fingerprinting
- Audit JSON output (`z-jail.audit/v1`)
- 17 test scenarios
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
