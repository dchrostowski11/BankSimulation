all:bankingClient bankingServer
bankingClient:bankingClient.c 
	gcc -g -Wall -pthread -fsanitize=address bankingClient.c -o bankingClient

bankingServer:bankingServer.c 
	gcc -g -pthread -fsanitize=address bankingServer.c -o bankingServer

clean:
	rm -rf bankingClient
	rm -rf bankingServer

