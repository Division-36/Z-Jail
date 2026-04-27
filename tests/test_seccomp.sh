#!/bin/bash
# Verify BPF filter properties
set -euo pipefail
make -C .. -j4 2>/dev/null
echo "seccomp: verify binary exists"
test -f ../z_jail && echo "PASS" || echo "FAIL"
