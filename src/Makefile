all: chat_server chat_client
CC=gcc
CCFLAGS=-fno-stack-protector -static

chat_server: chat_server.o room_manager.o
	$(CC) $(CCFLAGS) -o chat_server chat_server.o room_manager.o -lpthread

chat_server.o: chat_server.c
	$(CC) $(CCFLAGS) -c -o chat_server.o chat_server.c

room_manager.o: room_manager.c
	$(CC) $(CCFLAGS) -c -o room_manager.o room_manager.c

chat_client: chat_client.c
	$(CC) $(CCFLAGS) -o chat_client chat_client.c -lpthread -lseccomp

clean:
	rm chat_server chat_client *.o
