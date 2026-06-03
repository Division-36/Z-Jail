#!/bin/bash
set -euo pipefail
TMP=$(mktemp -d)
mkdir -p "$TMP/bin"
cp ../tests/build/hello_static "$TMP/bin/" 2>/dev/null || true
../z_jail --root="$TMP" --seccomp-enforce -- bin/hello_static 2>&1 && echo "PASS" || echo "FAIL"
rm -rf "$TMP"
