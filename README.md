# RemoteChatService

## Description

```
I made a super secure chat service, connect to chat server by remote client.
The flag is in the chat server.

(service address)
(binary & libc download link)
```

## How to run
In deploy folder,
```
# ./run.sh
```

## How to solve
1. Make and join the room.
2. In client, when receiving chat data from the server, it can overflow stack buffer. Exploit client, but you can not use executing system call.
3. On the server, it store chat data in the local stack. It also can overflow stack buffer. Explioit server by using client shellcode.
4. cat flag
