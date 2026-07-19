# Changelog

## [v1] - 2026-06-04

### Added
- Seven ordered isolation layers: rlimits, fd scrub,
  PR_SET_DUMPABLE, pivot_root, NO_NEW_PRIVS, cap drop,
  seccomp-BPF
- BLAKE2b content fingerprinting (RFC 7693)
- BPF whitelist with 24 syscalls + arg restrictions for mmap, mprotect, prlimit64
- Audit JSON output (`z-jail.audit/v1`)
- `--quiet`, `--help`, `--version` CLI flags
- install / uninstall / dist Makefile targets
- check / valgrind-check targets
- CI workflows: build, coverage, fuzz, lint, weekly, release
- bash, zsh, and fish shell completions
- Man page (`man/z_jail.1`)
- ADR documents (001–004)
- 18 test scenarios (standalone + sandbox)
- Release signing placeholder

### Changed
- Project renamed from Axiom-Jail to Z-Jail
- Rewritten BPF filter: KILL at end, nr chaining, arg jumps to KILL
- Test infrastructure: proper exit-code capture, standalone seccomp test
- Sandbox: setuid/setgid before capset fix
- Clone error handling: exit code 125, pipe fd leak fix

### Documentation
- README with architecture diagram, comparison table, layer descriptions
- docs/: ARCHITECTURE, SANDBOX, SECCOMP, AUDIT_SCHEMA, THREAT_MODEL,
  BLAKE2B, BENCHMARKS, BUILD
- Git hooks, Dockerfile

---

~135 commits across 6 weeks. Zero external dependencies, ~81 KiB binary.
