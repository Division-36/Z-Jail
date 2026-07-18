# BLAKE2b

## Why BLAKE2b
- ~2× faster than SHA-256 on x86_64
- ~120 LoC, no external dependency (no OpenSSL, no libsodium)
- RFC 7693

## Implementation

- `axiom_blake2b_init / _update / _final` — streaming API
- `axiom_blake2b_file()` — convenience for file hashing
- Digest size: 32 bytes (256 bits)
- Block size: 128 bytes
- Param block: digest_len=0x20, key_len=0, fanout=1, depth=1
  → `IV[0] ^= 0x0000000001010020ULL`

The `_update` path increments the byte counter `t` *before* each compression and
holds back the final block so that `_final` compresses it with the last-block
flag set (RFC 7693 §3.3). Output is canonical BLAKE2b-256: identical to
`b2sum -l 256` for inputs of any length, including multi-block files.

## Reference vectors

Single-block (canonical BLAKE2b-256):

```
empty:  0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8
abc:    bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319
```

Multi-block (exercise the block-boundary / counter logic):

```
128 x 0x00:        378d0caaaa3855f1b38693c1d6ef004fd118691c95c959d4efa950d6d6fcf7c1
200 x 'a':         6b6e59aaf00eb730cf93de53560846722184bbd92f8368c21ffa95380c2f9fe6
bytes 0x00..0xff:  39a7eb9fedc19aabc83425c6755dd90e6f9d0c804964a1f4aaeea3b9fb599835
```

All vectors are cross-checked against `b2sum -l 256`.

## Verify
```sh
./tests/build/blake2b_known
```
