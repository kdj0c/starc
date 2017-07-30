#!/usr/bin/env python

import sys

infile = sys.argv[1]
outfile = infile[:-5] + 'cfg'

with open(infile, 'r') as f:
    data = f.read()

#print(data.split('\n'))
lines = data.split('\n')

lines = lines[6:]

outdata = 'images = (\n'

for i in range(0, int(len(lines) / 7)):
    name = lines[7*i]
    x, y = lines[7*i + 2].strip().split()[1:]
    x = x.strip(',')
    w, h = lines[7*i + 3].strip().split()[1:]
    w = w.strip(',')
    outdata += '\t{\n'
    outdata += '\tname = "' + name + '"\n'
    outdata += '\tx = ' + x + '\n'
    outdata += '\ty = ' + y + '\n'
    outdata += '\twidth = ' + w + '\n'
    outdata += '\theight = ' + h + '\n'

    outdata += '\t},'
outdata += ')\n'

with open(outfile, 'w') as  f:
    f.write(outdata)
