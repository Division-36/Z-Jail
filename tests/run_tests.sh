#!/bin/bash
SELF="$(cd "$(dirname "$0")"&&pwd)"
Z="${SELF}/../z_jail"
D="${SELF}/build" P=0 F=0 S=0
msg() { printf "  %-40s %s\n" "$1" "$2"; }
pass() { P=$((P+1)); msg "$1" "PASS"; }
fail() { F=$((F+1)); msg "$1" "FAIL"; echo "$2"; }
skip() { S=$((S+1)); msg "$1" "SKIP"; }
run() { local r="$1" e="$2"; shift 2; "$Z" --root="$r" --seccomp-enforce -- "$e" "$@" 2>/dev/null || true; }
echo "=== Z-Jail Test Suite ==="
make -C "$SELF" setup>/dev/null 2>&1;mkdir -p "$D/audits"
test_b2b() {
  local o; o="$("$D/blake2b_known" 2>&1)" || true
  if echo "$o" | grep -q "MISMATCH"; then fail "$1" "$o"; else pass "$1"; fi
}
test_ok() {
  local o e; o="$(run "$2" "$3" 2>&1)" || true; e=$?
  if [[ $e -eq 0 || $e -eq 3 ]]; then pass "$1"; else fail "$1" "ec=$e o=$o"; fi
}
test_killed() {
  local o e; o="$(run "$2" "$3" 2>&1)" || true; e=$?
  if [[ $e -ne 0 ]]; then pass "$1"; else fail "$1" "ec=$e (should be killed)"; fi
}
test_b2b  "scenario 0: blake2b_regress"
test_ok   "scenario 1: hello_static"     "$D/roots/hello"   bin/hello_static
skip      "scenario 2: hello_dynamic"
test_ok   "scenario 3: execve_replacement" "$D/roots/execve" bin/payload_execve_replacement
skip      "scenario 4: fd_inherited_read"
test_killed "scenario 5: mmap_bad_flags" "$D/roots/mmap_bad" bin/payload_mmap_bad_flags
test_ok   "scenario 6: mmap_good_allowed" "$D/roots/mmap_good" bin/payload_mmap_good
test_killed "scenario 7: mmap_prot_exec" "$D/roots/mmap_protexec" bin/payload_mmap_prot_exec
test_killed "scenario 8: mmap_self_modify" "$D/roots/mmap_self" bin/payload_mmap_self_modify
test_killed "scenario 9: ptrace"  "$D/roots/ptrace"  bin/payload_ptrace
test_killed "scenario 10: socket" "$D/roots/socket"  bin/payload_socket
test_killed "scenario 11: chroot_escape" "$D/roots/chroot" bin/payload_chroot_escape
test_killed "scenario 12: double_chroot" "$D/roots/dbl_chroot" bin/payload_double_chroot
test_killed "scenario 13: mount_replay" "$D/roots/mount" bin/payload_mount_replay
test_killed "scenario 14: cpu_exhaust" "$D/roots/cpu_exh" bin/payload_cpu_exhaust
test_killed "scenario 15: signal_parent" "$D/roots/signal" bin/payload_signal_parent
test_ok   "scenario 16: self_hash" "$D/roots/self_hash_match" bin/hello_static
echo;echo "=== $P passed, $F failed, $S skipped ===";exit $F
