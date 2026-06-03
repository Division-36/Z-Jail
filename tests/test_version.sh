#!/bin/bash
set -euo pipefail
echo -n "version: "
../z_jail --version 2>&1
echo -n "help: "
../z_jail --help 2>&1 | head -1
echo "self-hash: $(../z_jail --self-hash=dummy 2>&1 || true)"
