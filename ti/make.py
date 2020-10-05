#!/usr/bin/env python

import sys
from subprocess import call


xas = ["/vol/git/xdt99/xas99.py", "-b", "-R", "-I", "/home/ralph/ti99/xdt99/lib/"]
xga = ["/vol/git/xdt99/xga99.py"]


if call(xas + ["menu.a99"], shell=False):
    sys.exit("ERROR: menu.a99")

with open("menu.bin", "rb") as f:
    data = f.read()

with open("menu.c", "w") as f:
    f.write("const uint8_t menu[] PROGMEM = {\n");
    for b in data[:-1]:
        f.write(hex(b) + ",\n");
    f.write(hex(data[-1]) + "};\n");


if call(xga + ["grom.gpl"], shell=False):
    sys.exit("ERROR: grom.a99")

with open("grom.gbc", "rb") as f:
    data = f.read()

with open("grom.c", "w") as f:
    f.write("const uint8_t grom[] PROGMEM = {\n");
    for b in data[:-1]:
        f.write(hex(b) + ",\n");
    f.write(hex(data[-1]) + "};\n");


if call(xas + ["help.a99"], shell=False):
    sys.exit("ERROR: help.a99")

with open("help.bin", "rb") as f:
    data = f.read()

with open("help.c", "w") as f:
    f.write("const uint8_t help[] PROGMEM = {\n");
    for b in data[:-1]:
        f.write(hex(b) + ",\n");
    f.write(hex(data[-1]) + "};\n");
