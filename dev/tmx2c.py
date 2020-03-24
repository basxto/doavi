#!/bin/env python3
from xml.dom import minidom
import os
import sys
filename = sys.argv[-1]
if filename.split('.')[-1] != 'tmx':
    print("Please give a tmx file", file=sys.stderr)
    exit(1)
cfilename = filename.split('.')[0] + '_tmap.c'
xmldoc = minidom.parse(filename)
tmxmap = xmldoc.getElementsByTagName('map')[0]

if tmxmap.attributes['width'].value != "10" and tmxmap.attributes['height'].value != "9":
    print("Map must be 9 tiles high and 10 tiles wide!", file=sys.stderr)
    exit(2)

file = open(cfilename,'w')
file.write('//Generated from ' + filename + '\n')

layers = xmldoc.getElementsByTagName('layer')
for l in layers:
    name = os.path.basename(filename).split('.')[0] + '_tmap_' + l.attributes['name'].value
    if l.attributes['name'].value == 'collision':
        # compress 8 values into one int
        # since it can only be 0 or 1
        file.write('const unsigned int ' + name + '[] = {')
        values = l.getElementsByTagName('data')[0].firstChild.nodeValue.replace('\n','').split(',')
        counter = 0
        hexnum = 0
        first = True
        for v in values:
            if counter == 0:
                if not first:
                    file.write(',')
                else:
                    first = False
            if v == '1':
                hexnum |= 1 << counter
            counter += 1
            if counter == 7:
                file.write(hex(hexnum))
                hexnum = 0
                counter = 0
        if counter != 0:
            file.write(hex(hexnum))
        file.write('};\n')
    else:
        file.write('const unsigned int ' + name + '[] = {' + l.getElementsByTagName('data')[0].firstChild.nodeValue + '};\n')
file.close() 
print('File written to ' + cfilename)