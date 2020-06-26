#!/bin/env python3
# Convert strings to c file
import argparse
import configparser

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('strings', metavar='strings.ini', help='Ini file with strings')
    parser.add_argument("--output", "-o", default="", help="Base name for generated c files")
    global args

    args = parser.parse_args()
    if args.output == "":
        args.output = '.'.join(args.strings.split('.')[:-1])
    else:
        args.output = '.'.join(args.output.split('.')[:-1])
    c = open(args.output+'.c', 'w')
    h = open(args.output+'.h', 'w')

    config = configparser.ConfigParser()
    config.read(args.strings)
    h.write("// Generated with ini2c.py\n#ifndef {0}_h\n#define {0}_h\n#define strlen(x) (sizeof(x) - 1)\nextern const unsigned char *text;\n".format(args.output))
    c.write("// Generated with ini2c.py\n#define strlen(x) (sizeof(x) - 1)\nconst unsigned char text[] = \"\";\n")
    for key in config['strings']:
        c.write('const unsigned char text_{0}[{2}] = "{1}";\n'.format(key, config['strings'][key], len(config['strings'][key])+1))
        h.write('extern const unsigned char text_{0}[{1}];\n'.format(key, len(config['strings'][key])+1))
    h.write("\n#endif")
    h.close()
    c.close()
    exit(0)

main()