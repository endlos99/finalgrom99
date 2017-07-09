#!/usr/bin/env python

# analyzes ROM/GROM image and shows menu entries
# Example: showcart.py <image> [...]


import sys
import os.path


def ordw(word):
    """word ord"""
    return ord(word[0]) << 8 | ord(word[1])


fns = sys.argv[1:]
if not fns:
    print "showcart.py -- show entries in cartridge images"
    print "usage: %s <image> ..." % sys.argv[0]
    sys.exit(0)

for fn in fns:
    if os.path.isdir(fn):
        continue
    print "%15s:" % fn,
    try:
        with open(fn, "rb") as fin:
            ws = fin.read()
        o = high = aa = auto = 0
        menu = 1
        tp = fn[fn.rfind(".") - 1].upper()
        try:
            while aa != 0xaa or (tp == "G" and menu == 0 and auto < 0x80):
                aa = ord(ws[o])
                auto = ord(ws[o + 1])
                menu = ordw(ws[o + 6:o + 8])
                o += 0x2000
        except IndexError:
            print "**MISSING >AA**"
            continue
        if auto >= 0x80 and tp == "G":
            print "**AUTOSTART**",
        elif 0x6000 <= menu:
            try:
                while menu != 0:
                    i = menu - 0x6000 + 4
                    len = ord(ws[i])
                    print ws[i + 1:i + 1 + len].rstrip(),
                    j = menu - 0x6000
                    menu = ordw(ws[j:j + 2])
            except IndexError:
                print "**BAD ENTRY**",
        else:
            print "**NO ENTRY**",
        if o > 0x2000:
            print "**HIGH GROM**",
        print
    except IOError as e:
        print "Error: %s, skipping" % e
        continue
