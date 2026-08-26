#!/usr/bin/env bash
# seccomp filter robustness gauge: reports the shipped filter's instruction
# count and largest 8-bit jump displacement (headroom to the cBPF jt/jf limit).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
CC=${CC:-gcc}
BIN="$(mktemp -d)/measure"
$CC -O2 -std=gnu11 -I "$ROOT/include" -Wl,--wrap=prctl \
    "$ROOT/_benchmarks/seccomp_robustness/measure.c" \
    "$ROOT/src/seccomp_bpf.c" "$ROOT/src/log.c" -o "$BIN"
"$BIN"
