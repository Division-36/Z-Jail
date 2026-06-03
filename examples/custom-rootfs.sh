#!/bin/bash
# Create a minimal rootfs and run a program inside it
set -euo pipefail
ROOT=$(mktemp -d)
mkdir -p "$ROOT/bin"
cp "$(which echo)" "$ROOT/bin/"
../z_jail --root="$ROOT" --seccomp-enforce -- bin/echo "hello from jail"
rm -rf "$ROOT"
