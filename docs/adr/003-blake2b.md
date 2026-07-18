# ADR 3: BLAKE2b for Content Fingerprinting
## Context
Need fast, self-contained content hashing for audit trail integrity.
## Decision
Use BLAKE2b (RFC 7693): ~120 LoC, ~2x SHA-256 speed, no external dep.
## Consequences
+ Fast, portable, simple implementation
+ Output is canonical BLAKE2b-256, verified against `b2sum -l 256` for
  single- and multi-block inputs, so audit fingerprints are independently
  verifiable without Z-Jail's own code
- Non-standard (SHA-256 is more universally expected)
