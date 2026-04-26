#!/bin/bash
# Example: run with network namespace
set -euo pipefail
make -C tests setup 2>/dev/null
../z_jail --root=tests/build/roots/hello --seccomp-enforce -- bin/hello_static
