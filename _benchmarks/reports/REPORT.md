# Axiom-Jail Benchmark Report

- Binary: `/mnt/d/Axioms/Z-Jail/axiom_jail` (127 KiB)
- Latency samples: **50**
- Memory  samples: **25**
- Fan-out samples: **15**

## 1. Latency overhead

Wall-clock time from `axiom_jail` invocation to the
point the child would `execve(2)`. Lower is better.

| metric | value |
|--------|-------|
| mean   | 72.70 ms |
| median | 73.60 ms |
| p95    | 89.11 ms |
| p99    | 98.24 ms |
| min    | 51.24 ms |
| max    | 103.63 ms |
| stdev  | 11.58 ms |

## 2. Memory footprint

Peak RSS of the warden process. Captured via /proc.

| metric | value |
|--------|-------|
| mean   | 1.43 MiB |
| median | 1.98 MiB |
| p95    | 1.98 MiB |
| max    | 1.98 MiB |

## 3. Fan-out stability

Per-process wall-clock time under M concurrent batches
of N workers. We report the coefficient of variation
(stdev / mean) per batch.

| batch | n  | mean | p95    | stdev  | cv %  |
|-------|----|------|--------|--------|-------|
|     0 |  5 | 60.90 ms | 65.09 ms | 4.02 ms |   6.61 |
|     1 |  5 | 55.25 ms | 60.96 ms | 3.98 ms |   7.20 |
|     2 |  5 | 53.23 ms | 59.83 ms | 4.44 ms |   8.34 |

## 4. Notes & caveats

- All measurements are taken on the host kernel, including
  namespace and capability setup. Numbers will differ across
  kernel versions and container runtimes.
- The 'fanout' benchmark intentionally stresses the
  audit-trail file system and seccomp filter installation;
  the seccomp BPF program is re-built per invocation and
  re-installed, so the per-invocation cost is visible in
  the numbers above.
- To regenerate the charts after re-running the harness,
  run `bench.sh charts` (requires matplotlib).
