#!/bin/env python3
# Convert strings to c file
import argparse
import configparser

def compress_string(string):
    tmp_string=""
    escape = False
    for char in string:
        if escape and char == 'n':
            char = '\n'
            escape = False
        if char != '\\' or escape:
            tmp_string += "\\x{:02X}".format(stringmap[char])
            escape = False
        else:
            escape = True
    return tmp_string



def main():
    global stringmap
    parser = argparse.ArgumentParser()
    parser.add_argument('strings', metavar='strings.ini', help='Ini file with strings')
    parser.add_argument('stringmap', metavar='stringmap.txt', help='Map for mapping strings to font')
    parser.add_argument("--output", "-o", default="", help="Base name for generated c files")
    global args

    args = parser.parse_args()
    if args.output == "":
        args.output = '.'.join(args.strings.split('.')[:-1])
    else:
        args.output = '.'.join(args.output.split('.')[:-1])

    stringmap = {'\0': 0, '\n': 1}
    next_index = 2

    with open(args.stringmap, 'r') as file:
        for char in file.read().replace('\n', ''):
            stringmap[char] = next_index
            # add a few handy aliases
            if(not char.lower() in stringmap):
                stringmap[char.lower()] = next_index
            if(char == '\'' and not '’' in stringmap):
                stringmap['’'] = next_index
            next_index += 1
    #print(stringmap)

    c = open(args.output+'.c', 'w')
    h = open(args.output+'.h', 'w')

    config = configparser.ConfigParser()
    config.read(args.strings)
    h.write("// Generated with ini2c.py\n#ifndef {0}_h\n#define {0}_h\n#define strlen(x) (sizeof(x) - 1)\nextern const unsigned char *text;\n".format(args.output))
    c.write("// Generated with ini2c.py\n#define strlen(x) (sizeof(x) - 1)\nconst unsigned char text[] = \"\";\n")
    for key in config['strings']:
        compressed = compress_string(config['strings'][key])
        length = len(compressed)//4 + 1 # \x00 is four characters
        c.write('const unsigned char text_{0}[{2}] = "{1}";\n'.format(key, compressed, length))
        h.write('extern const unsigned char text_{0}[{1}];\n'.format(key, length))
    h.write("\n#endif")
    h.close()
    c.close()
    exit(0)

main()