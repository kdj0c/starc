#!/usr/bin/env python

import sys
import json

infile = sys.argv[1]
outfile = infile[:-5] + 'json'

with open(infile, 'r') as f:
    data = f.read()

#print(data.split('\n'))
lines = data.split('\n')
lines = lines[6:]

outjson = []

for i in range(0, int(len(lines) / 7)):
    name = lines[7*i]
    x, y = lines[7*i + 2].strip().split()[1:]
    x = int(x.strip(','))
    y = int(y)
    w, h = lines[7*i + 3].strip().split()[1:]
    w = int(w.strip(','))
    h = int(h)
    outjson.append({"name": name, "x" : x, "y" : y, "w" : w, "h" : h})

outjson = {"images" : outjson}

with open(outfile, 'w') as  f:
    json.dump(outjson, f, indent=2)
