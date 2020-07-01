#!/bin/env python3
# Convert strings to c file
# Map characters like defined in stringmap
# Also compress with Digram Chain Encoding aka DiCE (byte pair encoding)
import argparse
import configparser

def compress_string(string):
    tmp_string=""
    for char in string:
        tmp_string += "\\x{:02X}".format(stringmap[char])
    return tmp_string

# for debugging
def decompress_string(string):
    tmp_string=""
    for i in range(0, len(string), 4):
        if int("0"+string[i+1:i+4], 16) < 0x80:
            tmp_string += stringmap_reverse[string[i:i+4]]
        else:
            tmp_string += string[i:i+4]
    return tmp_string

def most_used_pair(texts):
    byte_pairs = {}
    for pos in range(0, len(texts)-4, 4):
        # convert \x?? to hex
        chr1 = int("0"+texts[pos+1:pos+4], 16)
        chr2 = int("0"+texts[pos+5:pos+8], 16)
        # \n and ▶ must be avoided for now
        if chr1 > 1 and chr1 < 0x80 and chr2 > 1 and chr1 != stringmap['▶'] and chr2 != stringmap['▶']:
            pair = "\\x{:02X}\\x{:02X}".format(chr1, chr2)
            if pair in byte_pairs:
                byte_pairs[pair] += 1
            else:
                byte_pairs[pair] = 1
    # sort dictionary by value
    byte_pairs = {k: v for k, v in sorted(byte_pairs.items(), key=lambda pair: pair[1], reverse=True)}
    most_used = next(iter(byte_pairs))
    # print("'{}': {}".format(decompress_string(most_used), byte_pairs[most_used]))
    # it must be used more than once
    if byte_pairs[most_used] < 3:
        return ''
    return most_used

def main():
    global stringmap
    global stringmap_reverse
    global dictionary
    parser = argparse.ArgumentParser()
    parser.add_argument('strings', metavar='strings.ini', help='Ini file with strings')
    parser.add_argument('stringmap', metavar='stringmap.txt', help='Map for mapping strings to font')
    parser.add_argument('specialchars', metavar='specialchars.txt', help='Those characters get extra defines')
    parser.add_argument("--output", "-o", default="", help="Base name for generated c files")
    global args

    args = parser.parse_args()
    if args.output == "":
        args.output = '.'.join(args.strings.split('.')[:-1])
    else:
        args.output = '.'.join(args.output.split('.')[:-1])

    stringmap = {'\0': 0, '\n': 1}
    stringmap_reverse = {'\\x00': '\\0', '\\x01': '\\n'}
    dictionary = []
    texts = {}
    next_index = 2

    with open(args.stringmap, 'r') as file:
        for char in file.read().replace('\n', ''):
            stringmap[char] = next_index
            stringmap_reverse["\\x{:02X}".format(next_index)] = char
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

    # get all strings
    for key in config['strings']:
        texts[key] = compress_string(config['strings'][key].replace("\\n", "\n"))

    # build dictionary and compress texts
    # we can have 256 pairs in our dictionary
    offset = 0
    for i in range(0, 256):
        mup = most_used_pair("\\x00".join(texts.values()))
        if mup != '':
            dictionary.append(mup)
            for key in texts:
                texts[key] = texts[key].replace(mup,"\\x{:02X}".format(0x80 | offset))
            offset += 1

    h.write("// Generated with ini2c.py\n#ifndef {0}_h\n#define {0}_h\n#define strlen(x) (sizeof(x) - 1)\n#define specialchar_nl '\\x01'\n".format(args.output))
    c.write("// Generated with ini2c.py\n#define strlen(x) (sizeof(x) - 1)\nconst unsigned char text[{}] = \"{}\";\n".format(len("".join(dictionary))//4+1, "".join(dictionary)))
    with open(args.specialchars, 'r') as file:
        next_index = 1
        for char in file.read().replace('\n', ''):
            h.write("#define specialchar_{0} '\\x{1:02X}'\n".format(next_index, stringmap[char]))
            next_index += 1

    h.write("extern const unsigned char text[{}];\n".format(len("".join(dictionary))//4+1))
    for key in texts:
        compressed = texts[key]
        length = len(compressed)//4 + 1 # \x00 is four
        c.write('const unsigned char text_{0}[{2}] = "{1}";\n'.format(key, compressed, length))
        h.write('extern const unsigned char text_{0}[{1}];\n'.format(key, length))

    h.write("\n#endif")
    h.close()
    c.close()
    exit(0)

main()