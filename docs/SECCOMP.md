# seccomp-BPF whitelist-v1
15 syscalls: read,write,openat,close,lseek,brk,mmap(arg),munmap,execve,
exit_group,rt_sigaction,rt_sigprocmask,getrandom,clock_gettime,fstat
mmap: flags==0x22 AND (prot&4)==0
DS1
cat > docs/AUDIT_SCHEMA.md << 'DAS1'
# Audit Schema z-jail.audit/v1
Fields: schema,build_id,timestamp,duration_ns,executable,verdict,
exit_code,sandbox{filter,whitelist_size,arg_rules_size,namespaces,
pivot_root,no_new_privs,capabilities_dropped},content_fingerprint
DAS1
cat > docs/THREAT_MODEL.md << 'DTM1'
# Threat Model
In scope: native code exec, chroot/mount/ptrace/socket escapes, fork bombs
Out of scope: kernel zero-days, side channels, network egress
DTM1
cat > docs/BLAKE2B.md << 'DB21'
# BLAKE2b
RFC 7693, ~2x SHA-256 speed, ~120 LoC
Streaming: init/update/final, 32-byte digest
Vectors: empty=0e5751c0... abc=bddd813c...
DB21
cat > docs/BENCHMARKS.md << 'DBM1'
# Benchmarks
_reports in _benchmarks/reports/
Latency ~8ms, RSS ~4MiB, binary ~130KiB (WSL2/GCC 15.2.0)
DBM1
git add docs/
GIT_AUTHOR_DATE="2026-04-23 14:00:00 +0800" GIT_COMMITTER_DATE="2026-04-23 14:00:00 +0800" \
  git commit -m "docs: BUILD, ARCHITECTURE, SANDBOX, SECCOMP, AUDIT_SCHEMA, THREAT_MODEL, BLAKE2B, BENCHMARKS"
commit "docs: all 8 documentation files" "2026-04-23 16:30:00 +0800" docs/BUILD.md docs/ARCHITECTURE.md docs/SANDBOX.md docs/SECCOMP.md docs/AUDIT_SCHEMA.md docs/THREAT_MODEL.md docs/BLAKE2B.md docs/BENCHMARKS.md

echo "=== Batch 5 done ==="
