#!/bin/env python3
from xml.dom import minidom
import argparse
import os
import sys
import math

# long notation to bytes
def long2byte(value, bytepart):
    cvalues = ""
    if bytepart == 0:
        cvalues += "0x{0:02x}, ".format(value)
    else:
        cvalues += "{0:01x}, 0x{1:01x}".format((value>>4)&0xF, value&0xF)
    return cvalues

def short2halfbyte(value, bytepart):
    cvalues = ""
    if bytepart == 0:
        cvalues += "0x"
    cvalues += "{0:01x}".format(value&0xF)
    if bytepart == 1:
        cvalues += ", "
    return cvalues

def transform_value(v, byte, counter, cvalues, bytepart):
    value = 0
    # long grass in 4er blocks
    if counter >= 4:
        amount = math.floor(counter/4)
        for i in range(0, amount):
            cvalues += long2byte(0xC0 | (byte-2), bytepart)
        # let the tail half do the rest
        counter -= amount * 4
        if byte == 2:
            value = 0x0 | (byte-2)
        else:
            value = 0x80 | (byte)
        byte = v
    # short notation grass tiles
    elif byte >= 2 and byte <= 6:
        value = 0x0 | (byte-2)
        byte = v
    #tree tile pairs
    elif counter == 1 and byte == 10 and v == 11:
        value = 0x0 | 5
        byte = -1
    elif counter == 1 and byte == 12 and v == 13:
        value = 0x0 | 6
        byte = -1
    elif counter == 1 and byte == 14 and v == 15:
        value = 0x0 | 7
        byte = -1
    # long notation
    else:
        value = 0x80 | byte
        byte = v
    return (value, byte, counter, cvalues)

def compress(values):
    # 0x0 0XXX (half byte) often used (8)
    # 0x80 10XX XXXX (byte) regular tiles (64) - allows 256 8x8 tiles
    # 0xC0 11XX XXXX (byte) special tile mapping (64)
    counter = 0
    byte = -1
    # first (0) or second (1)
    bytepart = 0
    # compressed values
    cvalues = ""
    for v in values:
        # convert to interger, since they are all numbers
        v = int(v)
        if byte == -1:
            byte = v
            counter = 1
        elif v == byte:
            counter+=1
        else:
            value, byte, counter, cvalues = transform_value(v, byte, counter, cvalues, bytepart)

            for i in range(0, counter):
                # byte
                if value > 7:
                    cvalues += long2byte(value, bytepart)
                # half byte
                else:
                    cvalues += short2halfbyte(value, bytepart)
                    bytepart = (bytepart + 1) % 2
            # reset counter
            counter = 1

    # handle the last byte
    value, byte, counter, cvalues = transform_value(-1, byte, counter, cvalues, bytepart)

    for i in range(0, counter):
        # byte
        if value > 7:
            cvalues += long2byte(value, bytepart)
        # half byte
        else:
            cvalues += short2halfbyte(value, bytepart)
            bytepart = (bytepart + 1) % 2

    # add missing half byte
    if bytepart == 1:
        cvalues += "0"
    return cvalues

ignore = ['sprites']

parser = argparse.ArgumentParser()
parser.add_argument("--output", "-o", default="", metavar='map_tmap.c', help='C file representation')
parser.add_argument('tmx', metavar='map.tmx', nargs='+',help='Tiled map file')
parser.add_argument("--rom", "-r", default="", help="Address within the ROM (gets incremented if multiple files are given)")
parser.add_argument("--compress", "-c", default="no", help="Compress maps")
global args

args = parser.parse_args()

for filename in args.tmx:
        if filename.split('.')[-1] != 'tmx':
            print("Please give a .tmx file", file=sys.stderr)
            exit(1)
if args.output.split('.')[-1] != 'c':
    print("Please define a .c file as output", file=sys.stderr)
    exit(1)

rom = -1
if args.rom != "":
    rom = int(args.rom, 0)

for filename in args.tmx:
    #cfilename = filename.split('.')[0] + '_tmap.c'
    cfilename = args.output
    xmldoc = minidom.parse(filename)
    tmxmap = xmldoc.getElementsByTagName('map')[0]

    if tmxmap.attributes['width'].value != "10" and tmxmap.attributes['height'].value != "9":
        print("Map must be 9 tiles high and 10 tiles wide!", file=sys.stderr)
        exit(2)

    file = open(cfilename,'w')
    file.write('//Generated from ' + filename + ' by tmx2c\n')
    if args.compress != "no":
        file.write('//Compressed\n')

    chest = 0
    flame = 0
    properties = xmldoc.getElementsByTagName('property')
    # read custom variables
    for p in properties:
        if p.attributes['type'].value == "int":
            if p.attributes['name'].value == "chest":
                chest = int(p.attributes['value'].value)
            if p.attributes['name'].value == "flame":
                flame = int(p.attributes['value'].value)

    file.write('#define {}_tmap_chest '.format(os.path.basename(filename).split('.')[0]))
    if chest == 0:
        file.write('(0x00)\n')
    else:
        file.write('(0x{:02X})\n'.format(1<<(chest-1)))
    file.write('#define {}_tmap_flame '.format(os.path.basename(filename).split('.')[0]))
    if flame == 0:
        file.write('(0x00)\n')
    else:
        file.write('(0x{:02X})\n'.format(1<<(flame-1)))

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
                if args.compress != "no":
                    values = compress(values.split(','))
                file.write('const UINT8 ' + name + '[] = {' + values + '};\n')
                file.write('//{0} bytes\n'.format(len(values.split(','))))
                if rom != -1:
                    rom += len(values.split(','))
    file.close()
    print('File written to ' + cfilename)