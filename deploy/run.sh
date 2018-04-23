
docker build -t chat_server_container ./server
docker rm -f chat_server
docker run -d --name chat_server chat_server_container

docker build -t chat_client_container ./client
docker rm -f chat_client
docker run -d --link chat_server --name chat_client -p 13137:13137 chat_client_container
