# Benchmarks

Reports stored in `_benchmarks/reports/`.

## Metrics
- **Latency**: wall-clock time for one sandbox invocation (fork -> full
  isolation setup -> `execve` -> child exit -> reap), measured with a
  `getrusage`-based harness using `CLOCK_MONOTONIC`.
- **Memory**: peak resident set (`ru_maxrss` from `wait4`) of the launcher
  process for one invocation.

## Methodology (current)

- Host: Ubuntu 26.04 LTS, kernel `7.0.0-27-generic`, Intel i7-11800H (4 vCPUs), 3.8 GiB RAM (raw data in `_benchmarks/vm_results/`). A WSL2 run on the same hardware (`_benchmarks/reports/wsl_run/`) is ~2.3x slower for `z_jail` due to virtualization overhead.
- Uniform workload for every tool: a freestanding, statically linked
  binary whose entire body is `exit_group(0)` (no libc startup), so the
  process passes the `seccomp whitelist` policy and exits cleanly under
  all sandboxes. This isolates *setup* cost from workload cost.
- 50 measured samples per tool after 5 warm-up runs; all runs returned
  exit code 0.
- The mount-propagation bug found during benchmarking (see Notes) has been
  fixed in `src/sandbox.c`, so `z_jail` is measured directly with no
  workaround.

## Current numbers (native run, this codebase)

These are the native-host numbers for this codebase
(`_benchmarks/vm_results/`, Ubuntu 26.04, kernel 7.0.0, commit `c31fdcd`).
A WSL2 run on the same hardware is reported in the Notes section.

| Tool    | Latency mean +/- sd | 95% CI (ms)   | Peak RSS | Default seccomp |
|---------|---------------------|---------------|----------|-----------------|
| **z_jail** | 2.31 +/- 0.56 ms | [2.16, 2.47] | **1.61 MiB** | yes |
| bwrap   | 3.35 +/- 0.60 ms | [3.18, 3.51] | 2.32 MiB | no |
| nsjail  | 6.28 +/- 1.48 ms    | [5.87, 6.69]  | 7.86 MiB | yes |

z_jail has the smallest resident set and the lowest latency of the three
process-level sandboxes. bwrap applies no seccomp by default and is slightly
slower here; z_jail installs a seccomp whitelist, drops capabilities, and
does `pivot_root` on every run (the `MS_REC|MS_PRIVATE` remount from the
bug fix also walks the mount table, adding a small amount of setup latency).

Binary size (this codebase, measured in the native run): **~81 KiB**
unstripped (`-O2 -g`), **~33 KiB** stripped. Source lines of code
(`src` + `include`): ~800.

### Not directly comparable in this environment

- **gVisor (`runsc`)**: the released `runsc` binary segfaults on the WSL2
  kernel used here, so it could not be measured.
- **Firecracker**: isolates via a microVM and boots a guest kernel, so its
  startup is a VM cold-boot (published figure ~125 ms, <5 MiB overhead),
  not a fork/exec setup cost. It is intentionally excluded from the
  fork-to-exec latency table above to avoid an apples-to-oranges
  comparison.

## Notes & caveats

- Numbers are single-host and will differ across kernels, hardware, and
  virtualization (native vs WSL2); treat them as relative, not absolute.
- WSL2 run (same hardware, kernel `6.18.x-microsoft-standard-WSL2`): z_jail
  5.42 +/- 1.43 ms, bwrap 6.84 +/- 1.91 ms, nsjail 10.18 +/- 2.59 ms; z_jail
  peak RSS 1.55 MiB (see `_benchmarks/reports/wsl_run/`). Virtualization
  overhead, not the seccomp filter, drives the higher latency.
- Earlier reports in `_benchmarks/reports/` (mean latency ~72 ms, binary
  named `axiom_jail`) were taken with a different harness and do not
  reflect the current binary; re-measuring this codebase with the uniform
  harness above gives the numbers in the table above.
- Mount-propagation fix: `pivot_into()` in `src/sandbox.c` previously
  performed a recursive bind mount without first remarking `/` as
  `MS_PRIVATE`. On systemd hosts where `/` is `MS_SHARED`, repeated
  invocations leaked mounts into the host namespace (and could wedge the
  host). This is now fixed by issuing
  `mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)` before the bind
  mount; the recursive remount adds a small amount of setup latency.
