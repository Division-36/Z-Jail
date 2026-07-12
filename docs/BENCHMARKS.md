# Benchmarks

Reports stored in `_benchmarks/reports/`.

## Metrics
- **Latency**: wall-clock time for one sandbox invocation (fork -> full
  isolation setup -> `execve` -> child exit -> reap), measured with a
  `getrusage`-based harness using `CLOCK_MONOTONIC`.
- **Memory**: peak resident set (`ru_maxrss` from `wait4`) of the launcher
  process for one invocation.

## Methodology (current)

- Host: WSL2, kernel `6.18.x-microsoft-standard-WSL2`, Kali Linux.
- Uniform workload for every tool: a freestanding, statically linked
  binary whose entire body is `exit_group(0)` (no libc startup), so the
  process passes the `whitelist-v1` seccomp policy and exits cleanly under
  all sandboxes. This isolates *setup* cost from workload cost.
- 50 measured samples per tool after 5 warm-up runs; all runs returned
  exit code 0.
- The mount-propagation bug found during benchmarking (see Notes) has been
  fixed in `src/sandbox.c`, so `z_jail` is measured directly with no
  workaround.

## Current numbers (this codebase)

| Tool    | Latency mean +/- sd | 95% CI (ms)   | Peak RSS | Default seccomp |
|---------|---------------------|---------------|----------|-----------------|
| **z_jail** | 5.85 +/- 1.45 ms | [5.45, 6.25] | **1.62 MiB** | yes |
| bwrap   | **3.56 +/- 0.40 ms** | [3.45, 3.67] | 2.19 MiB | no |
| nsjail  | 8.98 +/- 1.68 ms    | [8.51, 9.45]  | 7.91 MiB | yes |

z_jail has the smallest resident set and a latency between bwrap and
nsjail. bwrap is fastest but applies no seccomp by default; z_jail installs
a seccomp whitelist, drops capabilities, and does `pivot_root` every run
(the `MS_REC|MS_PRIVATE` remount from the bug fix also walks the mount
table, adding some latency).

Binary size (this codebase): **~73 KiB** unstripped (`-O2 -g`),
**~28 KiB** stripped.

### Not directly comparable in this environment

- **gVisor (`runsc`)**: the released `runsc` binary segfaults on the WSL2
  kernel used here, so it could not be measured.
- **Firecracker**: isolates via a microVM and boots a guest kernel, so its
  startup is a VM cold-boot (published figure ~125 ms, <5 MiB overhead),
  not a fork/exec setup cost. It is intentionally excluded from the
  fork-to-exec latency table above to avoid an apples-to-oranges
  comparison.

## Notes & caveats

- Numbers are single-host (WSL2) and will differ across kernels and
  hardware; treat them as relative, not absolute.
- Earlier reports in `_benchmarks/reports/` (mean latency ~72 ms, binary
  named `axiom_jail`) were taken with a different harness and do not
  reflect the current binary; re-measuring this codebase with the uniform
  harness above gives the numbers in the table above. Note: Truthimatics
  is still part of the codebase (`src/truthimatics.c`); it was **not**
  removed.
- Mount-propagation fix: `pivot_into()` in `src/sandbox.c` previously
  performed a recursive bind mount without first remarking `/` as
  `MS_PRIVATE`. On systemd hosts where `/` is `MS_SHARED`, repeated
  invocations leaked mounts into the host namespace (and could wedge the
  host). This is now fixed by issuing
  `mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)` before the bind
  mount; the recursive remount adds a small amount of setup latency.
