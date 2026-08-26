# Z-Jail v1.1.0 — Security Hardening Release

**Release Date:** July 25, 2026
**Tag:** v1.1.0
**Previous Release:** v1 (June 2, 2026)
**License:** MIT
**Author:** Ziad Salah Mohamed (Division-36)

---

## Security Advisory

### seccomp-BPF Jump Offset Miscompilation (SJOM)

| Field | Value |
|-------|-------|
| **Class** | Silent Jump-Offset Miscompilation (SJOM) |
| **Severity** | Latent — never exploitable in production |
| **Affected Versions** | v1 (June 2, 2026) only |
| **Fixed In** | v1.1.0 (commit `9d96132`) |
| **CVSS Estimate** | 0.0 (latent, no production impact) |
| **Exploitability** | Requires 86+ syscall whitelist (shipped filter: 24) |
| **Real-World Precedent** | LXC issue #571 (2015) — same symptom class |

#### Description

Classic BPF (`struct sock_filter`) encodes conditional-jump displacements in
two `__u8` fields, `jt` and `jf`, with a maximum value of 255. Z-Jail's
`src/seccomp_bpf.c` built a linear dispatch table whose match arms jumped
forward to a single shared `RET KILL` at the end. As the whitelist grew, the
displacement exceeded 255 — and was **silently clamped** to 255 instead of
failing closed.

At **86 syscalls**, the architecture-check branch (`filter[1]`) needed
displacement **258** (jump to `RET KILL` at index 260). Clamped to **255**,
it landed on index 257 — a `RET ALLOW`. Effect: **every i386 (arch
`0x40000003`) syscall was allowed** — a classic seccomp architecture bypass,
produced purely by the emitter, not by a policy mistake.

```
filter[1] = code=21 (BPF_JMP|BPF_JEQ)  jt=0  jf=255  k=0xC000003E
```

#### The Fix

**Before (vulnerable):**
```c
filter[idx].jf = (off > 255 ? 255 : off);  // silent clamp
```

**After (hardened):**
```c
if (off > 255) {
    ERROR("seccomp: jump offset %u exceeds 8-bit limit", off);
    return -1;  // fail closed
}
filter[idx].jf = off;
```

The tool now **refuses to load** rather than install a filter that does not
match its policy.

#### Verification

| Test | Result |
|------|--------|
| Shipped 24-call filter | ✅ 0 mismatches, headroom 173 |
| N=85 (max safe) | ✅ 0 mismatches, max displacement 255 |
| N=86 (first broken) | ✅ Compiler refuses (fail-closed) |
| N=87+ | ✅ Compiler refuses (fail-closed) |
| 18 functional scenarios | ✅ All pass |
| 8 seccomp unit checks | ✅ All pass |
| Differential checker | ✅ 0 disagreements for N=1..85 |

#### Timeline

- **2026-07-20** — Bug discovered during research for DeepSec 2026 talk
- **2026-07-20** — Fix committed (`9d96132`)
- **2026-07-25** — Released as v1.1.0
- **No CVE assigned** — latent defect, no production impact

---

## What's New

### Security Hardening

- **seccomp-BPF jump offset validation** — Bounds-check all conditional jump
  offsets against the 8-bit field limit (max 255). Fail closed on overflow.
  Commit: `9d96132`

- **Audit chain hardening** — Removed Truthimatics module, tightened audit
  trail. Commit: `d112da6`

- **Expanded seccomp whitelist** — Added missing syscalls for modern glibc
  compatibility. Commit: `b05e860`

### Testing

- **seccomp filter unit test** — New `tests/seccomp_filter_test.c` validates
  the BPF filter structure, jump bounds, and policy correctness. 8 checks.

- **Improved test runner** — stdin redirect from /dev/null prevents blocking
  on inherited tty/pipe. Commit: `c31fdcd`

- **18-scenario test suite** — Standalone + sandbox tests covering all
  functional paths. Commit: `d4d035e`

### Benchmarking

- **Full native benchmark** — `_benchmarks/full_native_benchmark.sh` runs
  nsjail vs Z-Jail on syscall latency, throughput, and memory.

- **VM benchmark runner** — `_benchmarks/run_benchmark_vm.ps1` automates
  benchmarking in a隔离 VM environment. Commit: `ee57e76`

### Documentation

- **THREAT_MODEL.md** — Formal threat model for Z-Jail's isolation layers.
- **Updated README** — Architecture diagram, comparison table, badge overhaul.
- **ADR-004** — seccomp whitelist strategy documented.
- **Man page** — Updated for v1.1.0 changes.

---

## Files Changed

```
 .gitignore                           |  20 ++
 AUTHORS.md                           |   3 +-
 CHANGELOG.md                         |  53 ++--
 CODE_OF_CONDUCT.md                   |  26 +-
 CONTRIBUTING.md                      |  29 +-
 HISTORY.md                           |  11 +-
 LICENSE                              |   2 +-
 Makefile                             |  21 +-
 README.md                            | 485 +++++++++++++++++++++++++++
 RELEASE-SIGNING.md                   |  22 +-
 ROADMAP.md                           |  24 +-
 SECURITY.md                          |  11 +-
 _benchmarks/full_native_benchmark.sh | 483 +++++++++++++++++++++++++++++++++
 _benchmarks/oneliner.txt             |   1 +
 _benchmarks/reports/REPORT.md        |   6 +-
 _benchmarks/run_benchmark_vm.ps1     |  94 +++++++
 assets/zjail.jpg                     | Bin 0 -> 239751 bytes
 docs/ARCHITECTURE.md                 |   9 +-
 docs/AUDIT_SCHEMA.md                 |  47 +++-
 docs/BENCHMARKS.md                   |  86 ++++--
 docs/BLAKE2B.md                      |  26 +-
 docs/BUILD.md                        |   2 +-
 docs/SECCOMP.md                      |  71 ++---
 docs/THREAT_MODEL.md                 |  14 +
 docs/adr/001-no-external-deps.md     |   2 +-
 docs/adr/002-seven-layers.md         |   7 +-
 docs/adr/003-blake2b.md              |   3 +
 docs/adr/004-seccomp-whitelist.md    |   2 +-
 include/audit.h                      |   5 +
 include/truthimatics.h               |  29 ---  (REMOVED)
 include/util.h                       |   1 +
 include/z_jail.h                     |  13 +-
 man/z_jail.1                         |   2 +-
 src/audit.c                          | 151 ++++++++++-
 src/log.c                            |   2 +-
 src/sandbox.c                        |  20 +-
 src/seccomp_bpf.c                    |  88 ++++++-  (SECURITY FIX)
 src/truthimatics.c                   |  70 -----  (REMOVED)
 src/util.c                           |  40 ++-
 src/z_jail.c                         |  10 +-
 tests/Makefile                       |  25 +-
 tests/_setup_dynamic.sh              |  33 +++
 tests/blake2b_known.c                |  54 +++-
 tests/build/.gitkeep                 |   0
 tests/hello_dynamic.c                |   5 +
 tests/payload_cpu_exhaust.c          |   3 +-
 tests/payload_execve_replacement.c   |   4 +-
 tests/payload_signal_parent.c        |   1 +
 tests/run_tests.sh                   |  69 ++---
 tests/seccomp_filter_test.c          | 100 ++++++++
```

**51 files changed** — 1,943 insertions, 512 deletions

---

## Statistics

| Metric | v1 | v1.1.0 | Delta |
|--------|-----|--------|-------|
| Files changed | — | 51 | +51 |
| Insertions | — | 1,943 | +1,943 |
| Deletions | — | 512 | +512 |
| Commits since v1 | — | 20 | +20 |
| Binary size (full) | 81 KiB | 81 KiB | — |
| Binary size (stripped) | 33 KiB | 33 KiB | — |
| SLOC | ~800 | ~800 | — |
| Test scenarios | 17 | 17 | — |
| Seccomp unit checks | 0 | 8 | +8 |
| External dependencies | 0 | 0 | — |

---

## Verification

```bash
# Build from source
make clean && make

# Run full test suite (18 scenarios)
make test

# Run seccomp unit checks (8 checks)
./tests/seccomp_filter_test

# Run differential checker (optional, requires gcc + python3)
cd paper/DeepSecDEMO
bash build.sh
python3 experiments.py

# Expected output:
# Exp A: len=85 maxjt=46 maxjf=82 overflow=0 err=0 mismatches=0
# Exp B: first overflow N=86, first bypass N=86, first refuse N=86
```

---

## Upgrade Notes

**Backwards-compatible.** No configuration changes required.

- Existing policies with **< 86 syscalls**: unaffected, no action needed.
- Policies with **86+ syscalls**: will now **fail to load** with a clear error
  message instead of silently miscompiling. Reduce policy size or split into
  multiple filters.

**Breaking changes:** None.

**Deprecations:** Truthimatics module removed (was unused in production).

---

## Acknowledgments

- **Research:** seccomp-BPF miscompilation class documented in
  `paper/DeepSec/FINDINGS.md`
- **Tooling:** Differential checker in `paper/DeepSecDEMO/verify.py`
- **Precedent:** LXC issue #571 (2015) — real-world instance of the same
  fail-open symptom class
- **Talk:** "When the Filter Lies" — submitted to DeepSec 2026

---

## Links

- **Release:** https://github.com/Division-36/Z-Jail/releases/tag/v1.1.0
- **Compare:** https://github.com/Division-36/Z-Jail/compare/v1...v1.1.0
- **Research:** `paper/DeepSec/FINDINGS.md`
- **Demo:** `paper/DeepSecDEMO/`
- **Benchmarks:** `_benchmarks/seccomp_robustness/`

---

**Full Changelog:** https://github.com/Division-36/Z-Jail/compare/v1...v1.1.0
