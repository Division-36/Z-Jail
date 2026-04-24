# BLAKE2b

## Why BLAKE2b
- ~2× faster than SHA-256 on x86_64
- ~120 LoC, no external dependency (no OpenSSL, no libsodium)
- RFC 7693

## Implementation

- `xiom_blake2b_init / _update / _final` — streaming API
- `axiom_blake2b_file()` — convenience for file hashing
- Digest size: 32 bytes (256 bits)
- Block size: 128 bytes
- Param block: digest_len=0x20, key_len=0, fanout=1, depth=1
  → `IV[0] ^= 0x0000000001010020ULL`

## Reference vectors (RFC 7693 §2.5)

```
empty:  0e5751c026e543b2e417d46d4ddda751c8e9f88b0a26b5b1b55e7d40e2a4dabe
abc:    bddd813c634939b3b667e2c1d013f2c0b00014608a4ef8a6a5ec0bf66e7b2ea8
```

## Verify
```sh
python3 scripts/hash_known.py
./tests/build/blake2b_known
```
