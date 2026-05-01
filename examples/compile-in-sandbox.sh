#!/bin/bash
set -euo pipefail
# Compile a C file inside the sandbox
TMP=$(mktemp -d)
mkdir -p "$TMP/bin" "$TMP/tmp"
cp /usr/bin/gcc "$TMP/bin/"
"$(dirname "$0")/../z_jail" --root="$TMP" --seccomp-enforce -- bin/gcc -o /tmp/out "$1" 2>/dev/null || true
rm -rf "$TMP"
