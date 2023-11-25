#!/usr/bin/env python3
import sys

with open(sys.argv[1], 'wb') as o:
    for l in sys.stdin.readlines():
        o.write(int(l, 16).to_bytes(4, 'little'))
