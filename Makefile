all: chat_server chat_client

chat_server: chat_server.o room_manager.o
	gcc -o chat_server chat_server.o room_manager.o -lpthread

chat_server.o: chat_server.c
	gcc -c -o chat_server.o chat_server.c

room_manager.o: room_manager.c
	gcc -c -o room_manager.o room_manager.c

chat_client: chat_client.c
	gcc -o chat_client chat_client.c -lpthread -lseccomp

clean:
	rm chat_server chat_client *.o
