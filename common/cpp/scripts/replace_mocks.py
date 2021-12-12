#!/usr/bin/python

import sys
import re

fname = sys.argv[1]
fnameo = sys.argv[2]

with open(fname) as f:
    lines = f.readlines()

pattern = '.*(MOCK_.*?)\((.*?), (.*?)\((.*)\)\)'

f = open(fnameo, "w")

fulline = ""
in_mock = False
for line in lines:
    if not in_mock and re.match(r".*MOCK_.*", line):
        in_mock = True
    if in_mock:
        if fulline:
            fulline = fulline.rstrip() + " " + line.lstrip()
        else:
            fulline = line
        if not line.strip().endswith(";"):
            continue
    else:
        f.write(line)
        continue
    result = re.search(pattern, fulline)
    if result:
        result = result.groups()
        in_mock = False
    else:
        f.write(line)
        continue
    specifiers = ""
    if not result[1].endswith("_t"):
        specifiers = "override"
    if "CONST" in result[0]:
        if specifiers:
            specifiers = "const, " + specifiers
        else:
            specifiers = "const"
    ret = result[2].strip()
    if "," in ret:
        ret = "("+ret+")"
    s="MOCK_METHOD"+"("+ret+", "+result[1].strip()+", ("+result[3].strip()+"), ("+specifiers+"));"
    f.write(re.sub('MOCK_.*;', s , fulline))
    print (re.sub('MOCK_.*;', s , fulline))
    fulline = ""
f.close()