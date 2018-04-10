#include "chat_server.h"
#include "room_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 31337
//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT
#endif

void Error(char* str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

sem_t gSema;
sem_t gChatSema;
unsigned char gChatData[1024];
int gDataSize = 0;
int gSender = 0;
char bTrue = 1;
char bFalse = 0;



int Read(int fd, char* buffer, int dataLen)
{
    int readSize = 0;

    while (dataLen > 0)
    {
        readSize = recv(fd, buffer, dataLen, 0);
        if (readSize <= 0)
        {
            return readSize;
        }

        buffer += readSize;
        dataLen -= readSize;
    }

    return bTrue;
}

void* ChatClientHandler(void* arg)
{
    struct ClientInfo* ci = (struct ClientInfo*)arg;
    int *clientSock = ci->sock;
    unsigned int dataLen;
    char buffer[1024]; // maybe memory leak can be occurred for socket description number
    int ret;

    DEBUG_PRINT("Chat Client %x %d\n", clientSock, *clientSock);
    while ((ret = Read(*clientSock, (char*)&dataLen, 4)) == bTrue)
    {
        if (dataLen > 1024)
            dataLen = 1024;

        ret = Read(*clientSock, buffer, dataLen);
        if (ret <= 0)
            break;

        memcpy(gChatData, buffer, dataLen);
        gDataSize = dataLen;
        gSender = *clientSock;
        // or it can be 0xffff something like...
        if (strncmp(buffer, "bye", 3) == 0)
        {
            DEBUG_PRINT("bye received\n");
            send(*clientSock, "\xde\xad\xf0\x0d", 4, 0);
            ci->isClosed = bTrue;
            sem_post(&gChatSema);
            break;
        }
        sem_post(&gChatSema);
    }

    // Socket Closed
    if (ret == bFalse)
    {
        ci->isClosed = bTrue;
        DEBUG_PRINT("socket close chat server\n");
        //fflush(stdout);
        close(*clientSock);
        *clientSock = -1;
        sem_post(&gChatSema);
    }
    // recv error
    else if (ci->isClosed == bFalse)
    {
        perror("recv failed!\n");
        ci->isClosed = 1;
        sem_post(&gChatSema);
    }
}

int ChatHandler(int* sock1, int* sock2)
{
    int chatIdx = 0;
    unsigned char chat[65536];
    struct ClientInfo ci1;
    struct ClientInfo ci2;
    pthread_t threadId;
    sem_init(&gChatSema, 0, 0);

    ci1.sock = sock1;
    ci1.isClosed = bFalse;
    ci2.sock = sock2;
    ci2.isClosed = bFalse;

    DEBUG_PRINT("ChatHandler %x %d %x %d\n", sock1, *sock1, sock2, *sock2);
    if (pthread_create(&threadId, NULL, ChatClientHandler, (void*)&ci1) < 0)
        Error("pthread_create ChatClient 1 failed!");

    if (pthread_create(&threadId, NULL, ChatClientHandler, (void*)&ci2) < 0)
        Error("pthread_create ChatClient 2 failed!");

    while (ci1.isClosed == bFalse || ci2.isClosed == bFalse) // while sock1, sock2 alive
    {
        sem_wait(&gChatSema);
        memcpy(&chat[chatIdx], gChatData, gDataSize);

        if (gSender == *sock1)
        {
            // check socket alive
            if (*sock2 != -1 && ci2.isClosed == bFalse)
            {
                send(*sock2, "Remote: ", 8, 0);
                send(*sock2, &chat[chatIdx], strlen(&chat[chatIdx]), 0);
                send(*sock2, "\n", 1, 0);
            }
        }
        else if (gSender == *sock2)
        {
            // check socket alive
            if (*sock1 != -1 && ci1.isClosed == bFalse)
            {
                send(*sock1, "Remote: ", 8, 0);
                send(*sock1, &chat[chatIdx], strlen(&chat[chatIdx]), 0);
                send(*sock1, "\n", 1, 0);
            }
        }
        // TODO: if we can't assume the socket fd number, then we need to write
        // fd number at this point
        chatIdx += gDataSize;
    }

    DEBUG_PRINT("chat ended!\n");

    sem_destroy(&gChatSema);
}

int MakeRoom(int clientSock, char* roomName)
{
    char result = bTrue;
    struct Room* room = NULL;
    DEBUG_PRINT("make room %s\n", roomName);

    sem_wait(&gSema);
    // TODO: need to change 1, 0
    if (FindRoom(roomName) != NULL)
    {
        DEBUG_PRINT("room already exist!\n");
        result = bFalse;
    }
    else
    {
        DEBUG_PRINT("InsertRoom %s\n", roomName);
        if ((room = InsertRoom(clientSock, roomName)) == NULL)
            result = bFalse;
    }
    sem_post(&gSema);

    send(clientSock, &result, 1, 0);
    if (result == bTrue)
    {
        // wait 10 secs until someone join my room
        for (int i = 0; i < 10; i++)
        {
            DEBUG_PRINT("wait joiner\n");
            sleep(1);

            sem_wait(&gSema);
            if (room->connectionState == 1)
            {
                DEBUG_PRINT("joined!\n");
                room->connectionState = 2;
                sem_post(&gSema);
                break;
            }

            if (i == 9)
            {
                DEBUG_PRINT("No jointer... remove room\n");
                RemoveRoom(room);
                room = NULL;
                result = bFalse;
            }

            sem_post(&gSema);
        }

        send(clientSock, &result, 1, 0);
    }

    return result;
}

int JoinRoom(int clientSock, char* roomName)
{
    struct Room* room;
    char result = bFalse;
    DEBUG_PRINT("join room %s\n", roomName);

    sem_wait(&gSema);
    if (room = FindRoom(roomName))
    {
        DEBUG_PRINT("room found!\n");
        if (room->connectionState == 0)
        {
            DEBUG_PRINT("checked!\n");
            room->connectionState = 1;
            result = bTrue;
        }
    }
    sem_post(&gSema);

    send(clientSock, &result, 1, 0);

    if (result == bTrue)
    {
        DEBUG_PRINT("found room\n");
        result = bFalse;
        // wait owner state
        for (int i = 0; i < 3; i++)
        {
            sleep(1);
            if (room->connectionState == 2)
            {
                result = bTrue;
                break;
            }
        }

        if (result == bTrue)
        {
            int ownerSock = room->ownerSock;
            RemoveRoom(room);
            room = NULL;

            if (fork() == 0)
            {
                DEBUG_PRINT("forkd with sockets %x %d %x %d\n", &ownerSock, ownerSock, &clientSock, clientSock);
                ChatHandler(&ownerSock, &clientSock);

                if (ownerSock != -1)
                {
                    DEBUG_PRINT("ownerSock close\n");
                    close(ownerSock);
                }
                if (clientSock != -1)
                {
                    DEBUG_PRINT("clientSock close\n");
                    close(clientSock);
                }
            }
            else
            {
                close(ownerSock);
                close(clientSock);
            }
        }
    }
    else
    {
        DEBUG_PRINT("find room failed!\n");
    }

    return result;
}

void* ClientHandler(void* argv)
{
    int clientSock = *(int*)argv;
    char buffer[256];
    int readSize;
    int result;

    while ((readSize = recv(clientSock, &buffer, 255, 0)) > 0)
    {
        buffer[readSize] = '\0';
        if (buffer[0] == 1)
        {
            DEBUG_PRINT("make socket address %x %d \n", &clientSock, clientSock);
            if ((result = MakeRoom(clientSock, &buffer[1])) == bTrue)
            {
                DEBUG_PRINT("make success\n");
                break;
            }
        }
        else if (buffer[0] == 2)
        {
            DEBUG_PRINT("join socket address %x %d\n", &clientSock, clientSock);
            if ((result = JoinRoom(clientSock, &buffer[1])) == bTrue)
            {
                DEBUG_PRINT("join success\n");
                break;
            }
        }
    }

    DEBUG_PRINT("escape while!\n");

    if (readSize == 0)
    {
        DEBUG_PRINT("socket close %x\n", &clientSock);
        close(clientSock);
        clientSock = -1;
    }
    else if (result != bTrue)
    {
        perror("recv failed!");
    }
}

int main()
{
    int optval = 0x31337;
    int serverSock, clientSock;
    struct sockaddr_in addr;
    int addrLen = sizeof(addr);
    pthread_t threadId;

    sem_init(&gSema, 0, 1);

    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        Error("socket failed!");

    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) < 0)
        Error("setsockopt failed!");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(serverSock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        Error("bind failed!");

    if (listen(serverSock, 10) < 0)
        Error("listen failed!");

    while (1)
    {
        DEBUG_PRINT("wait client!!!\n");
        if ((clientSock = accept(serverSock, (struct sockaddr*)&addr, (socklen_t*)&addrLen)) < 0)
            Error("accept failed!");

        DEBUG_PRINT("client accepted!\n");

        if (pthread_create(&threadId, NULL, ClientHandler, (void*)&clientSock) < 0)
            Error("pthread_create failed!");
    }

    sem_destroy(&gSema);

    return 0;
}
