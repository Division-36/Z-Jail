#!/bin/bash
set -euo pipefail
echo "config: --version flag"
../z_jail --version 2>&1 | grep -q "Z-Jail" && echo "PASS" || echo "FAIL"
echo "config: --help (invalid)"
../z_jail --help 2>&1 && echo "FAIL" || echo "PASS (exit non-zero)"
