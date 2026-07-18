# Audit Schema

Version: `z-jail.audit/v1`

## Format

Single-line JSON written to `build/audits/<name>.audit.json`.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| schema | string | `"z-jail.audit/v1"` |
| build_id | string | `"Z-Jail/v1+dev"` |
| timestamp | int64 | Unix epoch seconds |
| duration_ns | int64 | Wall-clock runtime in ns |
| executable | string | Path to the target binary |
| verdict | string | `DETERMINISTIC`, `REJECT`, or `UNCERTAIN` |
| exit_code | int | Child exit code (or -signal) |
| sandbox.seccomp_filter | string | `"whitelist-v1"` |
| sandbox.seccomp_whitelist_size | int | Number of whitelist entries |
| sandbox.seccomp_arg_rules_size | int | Number of arg-restricted rules |
| sandbox.namespaces | array | `["mount","pid","net","ipc","uts"]` |
| sandbox.pivot_root | string | Root directory path |
| sandbox.no_new_privs | bool | Always `true` |
| sandbox.capabilities_dropped | bool | Always `true` |
| content_fingerprint | string | Canonical BLAKE2b-256 hex of the binary (equals `b2sum -l 256`) |
| prev_hash | string | BLAKE2b-256 hex of the previous record line; 64 zeros for the first (genesis) record |

## Integrity

The audit trail is hardened with three dependency-free mechanisms:

- **Canonical fingerprint.** `content_fingerprint` is the standard BLAKE2b-256
  of the target binary and is byte-for-byte reproducible with
  `b2sum -l 256 <binary>`. Any external tool can verify it independently.

- **Hash chain (`prev_hash`).** Each record commits to the previous one:
  `prev_hash = BLAKE2b-256(previous JSON line, without the trailing newline)`.
  The first record uses an all-zero genesis anchor. A verifier walks the file
  from the genesis and recomputes each line's digest; deletion, reordering, or
  in-place edits of any interior record break the chain and are detected.

- **Append-only storage.** The file is opened with `openat2(RESOLVE_NO_SYMLINKS)`
  (rejecting a symlink in any path component; falls back to `O_NOFOLLOW` on
  kernels < 5.6) and marked append-only (`FS_APPEND_FL`, i.e. `chattr +a`).
  Truncation and in-place overwrite then fail with `EPERM` for the owner; only
  a principal holding `CAP_LINUX_IMMUTABLE` can clear the flag.

### Verifying a trail

```sh
python3 - <<'PY'
import json, hashlib, sys
prev = '0'*64
for line in open('build/audits/<name>.audit.json'):
    line = line.rstrip('\n')
    if not line: continue
    assert json.loads(line)['prev_hash'] == prev, 'chain broken'
    prev = hashlib.blake2b(line.encode(), digest_size=32).hexdigest()
print('chain OK')
PY
```

### Residual limitation

The chain is tamper-evident relative to an externally anchored tip: a
privileged adversary (root) holds `CAP_LINUX_IMMUTABLE`/`CAP_DAC_OVERRIDE`, can
clear `+a`, and can recompute a self-consistent chain. Defending against the
host's own superuser requires an external trust anchor (per-record signing with
off-host keys, or remote log forwarding), which is outside the zero-dependency
design.
