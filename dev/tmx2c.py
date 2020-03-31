#!/bin/env python3
from xml.dom import minidom
import argparse
import os
import sys

ignore = ['sprites']

parser = argparse.ArgumentParser()
parser.add_argument('tmx', metavar='map.tmx', nargs='+',help='Tiled map file')
parser.add_argument("--rom", "-r", default="", help="Address within the ROM (gets incremented if multiple files are given)")
global args

args = parser.parse_args()

for filename in args.tmx:
        if filename.split('.')[-1] != 'tmx':
            print("Please give a .tmx file", file=sys.stderr)
            exit(1)

rom = -1
if args.rom != "":
    rom = int(args.rom, 0)

for filename in args.tmx:
    cfilename = filename.split('.')[0] + '_tmap.c'
    xmldoc = minidom.parse(filename)
    tmxmap = xmldoc.getElementsByTagName('map')[0]

    if tmxmap.attributes['width'].value != "10" and tmxmap.attributes['height'].value != "9":
        print("Map must be 9 tiles high and 10 tiles wide!", file=sys.stderr)
        exit(2)

    file = open(cfilename,'w')
    file.write('//Generated from ' + filename + ' by tmx2c\n')

    layers = xmldoc.getElementsByTagName('layer')
    for l in layers:
        if not l.attributes['name'].value in ignore:
            if rom != -1:
                file.write("__at (0x{0:02x}) ".format(rom))
            name = os.path.basename(filename).split('.')[0] + '_tmap_' + l.attributes['name'].value
            if l.attributes['name'].value == 'collision':
                # compress 8 values into one int
                # since it can only be 0 or 1
                file.write('const UINT8 ' + name + '[] = {')
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
                    if counter == 8:
                        if rom != -1:
                            rom += 1
                        file.write(hex(hexnum))
                        hexnum = 0
                        counter = 0
                if counter != 0:
                    if rom != -1:
                        rom += 1
                    file.write(hex(hexnum))
                file.write('};\n')
            else:
                values = l.getElementsByTagName('data')[0].firstChild.nodeValue
                file.write('const UINT8 ' + name + '[] = {' + values + '};\n')
                if rom != -1:
                    rom += len(values.split(','))
    file.close() 
    print('File written to ' + cfilename)