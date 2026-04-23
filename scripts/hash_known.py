#!/usr/bin/env python3
import hashlib
e=hashlib.blake2b(b'',digest_size=32).hexdigest()
a=hashlib.blake2b(b'abc',digest_size=32).hexdigest()
print(f"empty: {e}\nabc:   {a}")
assert e=="0e5751c026e543b2e417d46d4ddda751c8e9f88b0a26b5b1b55e7d40e2a4dabe"
assert a=="bddd813c634939b3b667e2c1d013f2c0b00014608a4ef8a6a5ec0bf66e7b2ea8"
print("OK")
