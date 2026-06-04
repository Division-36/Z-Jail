#!/bin/bash
SELF="$(cd "$(dirname "$0")"&&pwd)"
Z="${SELF}/../z_jail"
D="${SELF}/build" P=0 F=0 S=0
msg() { printf "  %-40s %s\n" "$1" "$2"; }
pass() { P=$((P+1)); msg "$1" "PASS"; }
fail() { F=$((F+1)); msg "$1" "FAIL"; echo "$2"; }
skip() { S=$((S+1)); msg "$1" "SKIP"; }
pe() { "$Z" --root="$1" --seccomp-enforce -- "$2" 2>/dev/null; return $?; }
echo "=== Z-Jail Test Suite ==="
make -C "$SELF" setup>/dev/null 2>&1;mkdir -p "$D/audits"
test_b2b() {
  local o; o="$("$D/blake2b_known" 2>&1)" || true
  if echo "$o" | grep -q "MISMATCH"; then fail "$1" "$o"; else pass "$1"; fi
}
test_ok() {
  pe "$2" "$3"; local e=$?
  if [[ $e -eq 0 || $e -eq 3 ]]; then pass "$1"; else fail "$1" "ec=$e"; fi
}
test_killed() {
  pe "$2" "$3"; local e=$?
  if [[ $e -ne 0 ]]; then pass "$1"; else fail "$1" "ec=$e (should be killed)"; fi
}
test_b2b  "scenario 0: blake2b_regress"
test_sec() {
  "$D/seccomp_filter_test" 2>&1; local e=$?
  if [[ $e -eq 0 ]]; then pass "$1"; else fail "$1" "ec=$e"; fi
}
test_sec "scenario 1: seccomp_filter"
test_ok   "scenario 2: hello_static"     "$D/roots/hello"   bin/hello_static
skip      "scenario 3: hello_dynamic"
test_ok   "scenario 4: execve_replacement" "$D/roots/execve" bin/payload_execve_replacement
skip      "scenario 5: fd_inherited_read"
test_killed "scenario 6: mmap_bad_flags" "$D/roots/mmap_bad" bin/payload_mmap_bad_flags
test_ok   "scenario 7: mmap_good_allowed" "$D/roots/mmap_good" bin/payload_mmap_good
test_killed "scenario 8: mmap_prot_exec" "$D/roots/mmap_protexec" bin/payload_mmap_prot_exec
test_killed "scenario 9: mmap_self_modify" "$D/roots/mmap_self" bin/payload_mmap_self_modify
test_killed "scenario 10: ptrace"  "$D/roots/ptrace"  bin/payload_ptrace
test_killed "scenario 11: socket" "$D/roots/socket"  bin/payload_socket
test_killed "scenario 12: chroot_escape" "$D/roots/chroot" bin/payload_chroot_escape
test_killed "scenario 13: double_chroot" "$D/roots/dbl_chroot" bin/payload_double_chroot
test_killed "scenario 14: mount_replay" "$D/roots/mount" bin/payload_mount_replay
test_killed "scenario 15: cpu_exhaust" "$D/roots/cpu_exh" bin/payload_cpu_exhaust
test_killed "scenario 16: signal_parent" "$D/roots/signal" bin/payload_signal_parent
test_ok   "scenario 17: self_hash" "$D/roots/self_hash_match" bin/hello_static
echo;echo "=== $P passed, $F failed, $S skipped ===";exit $F
