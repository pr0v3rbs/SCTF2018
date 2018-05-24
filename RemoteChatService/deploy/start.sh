#!/bin/bash
# client xinetd
/etc/init.d/xinetd start;
# server
while ( true ); do
    su chat_server -c /home/chat_server/chat_server
done
