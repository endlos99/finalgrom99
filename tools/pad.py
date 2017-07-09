#!/usr/bin/env python

# pads file to given size or 8192 bytes if no size is given

import sys
import os.path


fns = sys.argv[1:]
if not fns:
    print "pad.py -- pad image file to multiple of 8K"
    print "usage: %s <file> ..." % sys.argv[0]
    sys.exit(0)

for fn in fns:
    # read input file and pad to multiple of 8K
    try:
        with open(fn, "rb") as fin:
            size = len(fin.read())
        with open(fn, "ab") as fout:
            fout.write("\xff" * (-size % 8192))
    except IOError as e:
        print "Error: %s, skipping" % e
        continue
