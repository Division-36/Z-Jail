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
| content_fingerprint | string | BLAKE2b-256 hex of the binary |
