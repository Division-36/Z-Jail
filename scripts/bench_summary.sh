#!/bin/bash
set -euo pipefail
for f in _benchmarks/reports/*.txt; do
    echo "=== $(basename $f) ==="
    awk '{sum+=$1;n++}END{print "avg: "sum/n" ns"}' "$f"
done
