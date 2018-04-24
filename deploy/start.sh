#!/bin/bash
# client xinetd
/etc/init.d/xinetd start;
# server
cd /home/chat_server/
while ( true ); do
    ./chat_server
done
