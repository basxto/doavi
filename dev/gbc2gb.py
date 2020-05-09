#!/bin/env python3
# Transforms image from colorful CGB to 4 color DMG
# and exports the previous palette
# Used palette is https://lospec.com/palette-list/dirtyboy
# This needs https://github.com/drj11/pypng
import png
import re
import os
import sys
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('image', metavar='image_gbc.png', nargs='+',help='8bit PNG image')
    parser.add_argument("--palette", "-p", default="", help="Palette file (default: image.pal)")
    parser.add_argument("--output", "-o", default="", help="Output image (default: image_gb.png)")
    global args

    args = parser.parse_args()
    if args.palette == "":
        args.palette = re.sub("(_gbc)?\.png", ".pal", args.image[0])
    if args.output == "":
        args.output = re.sub("(_gbc)?\.png", "_gb.png", args.image[0])

    # 4 color palette
    palette = [(0x1f,0x1f,0x1f),(0x4d,0x53,0x3c),(0x8b,0x95,0x6d),(0xc4,0xcf,0xa1)]
    # read original image
    r=png.Reader(filename=args.image[0])
    original = r.read()
    # write the palette file
    f = open(args.palette, 'wb')
    pal = original[3]['palette']
    for col in pal:
        r = int(col[0]/8)
        g = int(col[1]/8)
        b = int(col[2]/8)
        rgb555 = r<<10 | g<<5 | b
        f.write(bytes([rgb555>>8, rgb555&0xFF]))
        #print("RGB({0},{1},{2}) = 0x{3:04X}".format(r, g ,b , rgb555))
    f.close()

    # write new image
    w = png.Writer(original[0], original[1], palette=palette, bitdepth=8)
    f = open(args.output, 'wb')
    # mod4 every pixel
    data=map(lambda x: list(map(lambda y: y%4, x)), original[2])
    
    w.write(f, data)
    f.close()
main()