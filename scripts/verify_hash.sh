#!/bin/bash
set -euo pipefail
EXPECTED="$1"
BINARY="${2:-./z_jail}"
HASH=$(python3 scripts/get_hash.py "$BINARY")
if [ "$HASH" = "$EXPECTED" ]; then echo "OK"; else echo "MISMATCH: got $HASH"; exit 1; fi
