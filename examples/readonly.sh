#!/bin/bash
set -euo pipefail
make -C tests setup 2>/dev/null
# Run in sandbox, no --seccomp-enforce for wider compat
"$(dirname "$0")/../z_jail" --root=tests/build/roots/hello -- bin/hello_static
