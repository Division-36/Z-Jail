#!/bin/bash
set -euo pipefail
VERSION="${1:-v1}"
echo "Building $VERSION"
make clean && make
echo "Tagging $VERSION"
git tag -a "$VERSION" -m "Release $VERSION"
