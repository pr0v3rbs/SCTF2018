# RemoteChatService

## Description

```
I made a super secure chat service, connects to chat server by using remote client.
The flag is in the chat server.

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
3. On the server, it store chat data in the local stack. It also can overflow stack buffer.
4. read flag.
