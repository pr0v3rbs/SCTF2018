#!/usr/bin/env python

import sys
import struct
import os

up = lambda x : struct.unpack('<L', x)[0]

SYN_SIG = '\x00\x00\x00\x00\xa0\x02\x72\x10'
FIN_SIG = '\x80\x11\x00\xe5\x82\x62'
HELLO_SIG = '\x00\xaf\x01\x00\x00\xab\x03\x03'
RESPONSE_SIG = '\x00\x0c\x29\xac\x31\xb8\x08\x00\x45\x00\x00\x35'

def check_response(packets):
    idx = packets.find(RESPONSE_SIG)
    if packets[idx + 16*4 - 4] == '1':
        return True
    else:
        return False

def extract_data(packets):
    global total_size
    data_size = up(packets[4:8])
    data = packets[8:32]

    # check whether server response is '1'
    if check_response(packets):
        return data[:data_size]
    else:
        return ''

def extract_file(session):
    global total_size
    data = ''
    hello_index = -1

    # split with each Client Hello and response
    prev_seq = -1
    seq = -1
    while True:
        hello_index = session.find(HELLO_SIG, hello_index + 1)
        if hello_index == -1:
            break
        next_index = session.find(HELLO_SIG, hello_index + 1)
        seq = session[hello_index - 16*2 + 1:hello_index - 16*2 + 5]
        if prev_seq != -1 and prev_seq == seq: # skip retransmission packet
            continue
        prev_seq = seq
        data += extract_data(session[hello_index + len(HELLO_SIG) : next_index])

    return data

def main():
    pcapfile = 'problem.pcap'
    c = 'host 192.168.0.107'
    s = 'host 192.168.0.128'
    os.system('tcpdump -r {} -w {} "(src {} && dst {}) || (src {} && dst {})"'.format(pcapfile, "filtered_" + pcapfile, c, s, s, c))
    data = open("filtered_" + pcapfile).read()

    file_index = 0
    syn_index = -1

    # split with each session
    while True:
        syn_index = data.find(SYN_SIG, syn_index + 1)
        if syn_index != -1:
            fin_index = data.find(FIN_SIG, syn_index)
            file_data = extract_file(data[syn_index:fin_index])
            if file_data[:16].find('PNG') != -1:
                ext = '.png'
            elif file_data[:16].find('\x49\x49\x2A\x00') != -1:
                ext = '.tif'
            elif file_data[:16].find('GIF89a') != -1:
                ext = '.gif'
            elif file_data[:16].find('JFIF') != -1:
                ext = '.jpg'
            elif file_data[:16].find('BM6') != -1:
                ext = '.bmp'
            else:
                ext = ''
            open(str(file_index).rjust(2, '0') + ext, 'w').write(file_data)
            file_index += 1
        else:
            break

if __name__ == '__main__':
    main()
