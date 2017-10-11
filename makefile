make: client.c server.c
	gcc server.c -o server
	gcc client.c -o client

remove: client server
	rm client
	rm server

