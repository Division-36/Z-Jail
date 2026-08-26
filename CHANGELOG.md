# Changelog

## [v1.1.0] - 2026-07-25

### Security
- **Critical:** Fixed seccomp-BPF jump offset miscompilation (SJOM)
  - Conditional jumps exceeding 8-bit field (max 255) were silently clamped
  - At 86 syscalls, arch-check lands on RET ALLOW instead of RET KILL
  - Fix: bounds check returns -1 on overflow, tool refuses to run
  - Commit: 9d96132

### Added
- seccomp filter unit test (`tests/seccomp_filter_test.c`)
- Full native benchmark script (`_benchmarks/full_native_benchmark.sh`)
- VM benchmark runner (`_benchmarks/run_benchmark_vm.ps1`)
- THREAT_MODEL documentation
- ADR-004: seccomp whitelist strategy

### Changed
- Removed Truthimatics module (include/truthimatics.h, src/truthimatics.c)
- Expanded seccomp whitelist-v1 for modern glibc
- Aligned docs with measured build (81 KiB / 33 KiB / 800 SLOC)
- Reverted license to MIT

### Fixed
- Benchmark harness: apt lock wait, bwrap root under /tmp, timeout guards
- nsjail VM execution: --disable_clone_newuser
- Test runner: stdin redirect from /dev/null
- Stale 'Axiom-Jail' identity in benchmark reports

---

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
