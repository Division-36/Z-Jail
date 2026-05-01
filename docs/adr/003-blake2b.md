# ADR 3: BLAKE2b for Content Fingerprinting
## Context
Need fast, self-contained content hashing for audit trail integrity.
## Decision
Use BLAKE2b (RFC 7693): ~120 LoC, ~2x SHA-256 speed, no external dep.
## Consequences
+ Fast, portable, simple implementation
- Non-standard (SHA-256 is more universally expected)
