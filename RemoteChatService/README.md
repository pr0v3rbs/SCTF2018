# RemoteChatService

## Description

```
I made a super secure chat service, connects to chat server by using the remote client.
You need to exploit the chat server to get the flag.

(service address)
(binary download link)
```

## How to run
In deploy folder
```
# ./run.sh
```

## How to solve
1. Make and join the room.
2. In client, when receiving chat data from the server, it can overflow local stack buffer.
3. On the server, when copy the chat data in the local stack, it also can overflow stack buffer.
4. It needs to exploit remote client first, to trigger the vulnerability in the chat server.
5. Read the flag.
