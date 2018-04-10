#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <seccomp.h>

int bTrue = 1;
int bFalse = 0;
int gServerSock = 0;

void Error(char* str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

void PrintLogo()
{
    puts(" _____                      _                  ");
    puts("|  __ \\                    | |                 ");
    puts("| |__) |___ _ __ ___   ___ | |_ ___            ");
    puts("|  _  // _ \\ '_ ` _ \\ / _ \\| __/ _ \\           ");
    puts("| | \\ \\  __/ | | | | | (_) | ||  __/           ");
    puts("|_|  \\_\\___|_|_|_|_|_|\\___/ \\__\\___|           ");
    puts("             / ____| |         | |             ");
    puts("            | |    | |__   __ _| |_            ");
    puts("            | |    | '_ \\ / _` | __|           ");
    puts("            | |____| | | | (_| | |_            ");
    puts("             \\_____|_| |_|\\__,_|\\__|_          ");
    puts("             / ____|               (_)         ");
    puts("            | (___   ___ _ ____   ___  ___ ___ ");
    puts("             \\___ \\ / _ \\ '__\\ \\ / / |/ __/ _ \\");
    puts("             ____) |  __/ |   \\ V /| | (_|  __/");
    puts("            |_____/ \\___|_|    \\_/ |_|\\___\\___|");
}

void PrintMenu()
{
    puts("1. Make room");
    puts("2. Join room");
    puts("3. Exit");
}

int ReadLine(char* buffer, int maxLen)
{
    int readSize = 0;
    int readLen = 0;

    while ((readSize = read(0, buffer, 1)) > 0)
    {
        if (++readLen == maxLen)
            break;
        if (*buffer == '\n')
        {
            *buffer = '\0';
            break;
        }
        ++buffer;
    }

    return readLen;
}

void Connect()
{
    struct sockaddr_in addr;

    if ((gServerSock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        Error("socket failed!");

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(31337);

    if (connect(gServerSock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        Error("connect failed!");
}

void* ChatReceiver(void* arg)
{
    int readSize = 0;
    unsigned char buffer[256];

    // BOF vulnerability!!!
    while ((readSize = recv(gServerSock, buffer, 1024, 0)) > 0)
    {
        if (buffer[0] == 0xde && buffer[1] == 0xad && buffer[2] == 0xf0 && buffer[3] == 0x0d)
        {
            break;
        }
        write(1, buffer, readSize);
    }
}

void Chat()
{
    char buffer[1028];
    int readLen;
    pthread_t threadId;

    if (pthread_create(&threadId, NULL, ChatReceiver, NULL) < 0)
        Error("pthread_create ChatReceiver failed!");

    puts("=====Chat Start=====");

    while (bTrue)
    {
        readLen = ReadLine(&buffer[4], 1024);
        *((int*)&buffer[0]) = readLen;
        send(gServerSock, buffer, readLen + 4, 0);
        if (strncmp(&buffer[4], "bye", 3) == 0)
            break;
    }

    pthread_join(threadId, NULL);

    puts("=====Chat Ended=====");

    close(gServerSock);
    gServerSock = 0;
}

int MakeRoom()
{
    char buffer[256];
    int readLen = 0;

    write(1, "Room Name>>", strlen("Room Name>>"));
    readLen = ReadLine(&buffer[1], 255);

    buffer[0] = 1;
    send(gServerSock, buffer, readLen + 1, 0);
    recv(gServerSock, buffer, 1, 0);

    if (buffer[0] == bTrue)
    {
        puts("wait 10 seconds...");
        recv(gServerSock, buffer, 1, 0);
        if (buffer[0] == bTrue)
        {
            puts("Someone joined!");
            Chat();
        }
        else
        {
            puts("Anyone not joined");
        }
    }
    else
    {
        puts("Make room failed");
    }
}

int JoinRoom()
{
    char buffer[256];
    int readLen = 0;

    write(1, "Room Name>>", strlen("Room Name>>"));
    readLen = ReadLine(&buffer[1], 255);

    buffer[0] = 2;
    send(gServerSock, buffer, readLen + 1, 0);
    read(gServerSock, buffer, 1);

    if (buffer[0] == bTrue)
    {
        puts("Join room succeeded");
        Chat();
    }
    else
    {
        puts("Join room failed");
    }
}

int main()
{
    char buf[10];
    int menu = 0;
    int isExit = bFalse;
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_KILL);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(connect), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(send), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(recv), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);

    seccomp_load(ctx);

    PrintLogo();

    while (isExit == bFalse)
    {
        PrintMenu();
        ReadLine(buf, 10);
        menu = atoi(buf);

        if (menu < 3 && gServerSock == 0)
            Connect();

        switch (menu)
        {
            case 1:
                if (gServerSock)
                    MakeRoom();
                else
                    puts("not connected");
                break;
            case 2:
                if (gServerSock)
                    JoinRoom();
                else
                    puts("not connected");
                break;
            case 3:
                isExit = bTrue;
                break;
            default:
                puts("Invalid Menu");
                break;
        }
    }

    puts("Bye");

    return 0;
}
