#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
Z=./z_jail
make -C tests setup 2>/dev/null
R=tests/build/roots/hello
echo "bench: latency x 10"
for i in $(seq 1 10); do
    t0=$(date +%s%N)
    $Z --root="$R" --seccomp-enforce -- bin/hello_static 2>/dev/null || true
    t1=$(date +%s%N)
    echo "$((t1-t0)) ns"
done | tee _benchmarks/reports/latency_$(date +%Y%m%d_%H%M%S).txt
