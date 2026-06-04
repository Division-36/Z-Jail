#!/bin/bash
# Set up dynamic test roots: copy ld-linux and shared libs into chroots.
# Run after `make all` from tests/ directory.
set -e
SELF="$(cd "$(dirname "$0")"&&pwd)"
cd "$SELF"

setup_root() {
    local root="$1" binary="$2"
    local rootdir="build/roots/$root"
    mkdir -p "$rootdir/bin"
    cp "build/$binary" "$rootdir/bin/" 2>/dev/null || true

    local interp libs lib
    interp=$(readelf -l "build/$binary" 2>/dev/null | grep "interpreter" | sed 's/.*: //;s/]//')
    libs=$(ldd "build/$binary" 2>/dev/null | grep -E '=> /' | awk '{print $3}')

    if [ -n "$interp" ]; then
        local d; d=$(dirname "$interp")
        mkdir -p "$rootdir$d"
        cp -L "$interp" "$rootdir$d/"
    fi
    for lib in $libs; do
        [ -z "$lib" ] && continue
        local d; d=$(dirname "$lib")
        mkdir -p "$rootdir$d"
        cp -L "$lib" "$rootdir$d/"
    done
}

setup_root hello_dynamic hello_dynamic
setup_root fd_inh payload_fd_inherited_read_dyn
echo "  Dynamic test roots set up."
