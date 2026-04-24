#!/usr/bin/env python3
import sys,hashlib
data=open(sys.argv[1],'rb').read()
print(hashlib.blake2b(data,digest_size=32).hexdigest())
