#!/usr/bin/env python

# inverts (and un-inverts) cartridge image files,
# won't overwrite original image files
# Example: invert.py game9.bin other9.bin

import sys
import os.path


fns = sys.argv[1:]
if not fns:
    print "invert.py -- TI 99 cartridge image inverter"
    print "usage: %s <file> ..." % sys.argv[0]
    sys.exit(0)

for fn in fns:
    # read input file and pad to multiple of 8K
    try:
        with open(fn, "rb") as fin:
            data = fin.read()
    except IOError as e:
        print "Error: %s, skipping" % e
        continue
    data += "\x00" * (-len(data) % 8192)

    # output file name is input file name plus "_i"
    basename = os.path.basename(fn)
    name, ext = os.path.splitext(basename)

    # reserve 8K chunks and write to output file
    try:
        with open(name + "_i" + ext, "wb") as fout:
            chunks = [data[i:i + 8192]
                      for i in xrange(0, len(data), 8192)]
            fout.write("".join(reversed(chunks)))
    except IOError as e:
        print "Error: %s, skipping" % e
