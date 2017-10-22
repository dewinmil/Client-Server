make: client.c server.c functions.c
	gcc -c functions.c
	gcc functions.o server.c -o server
	gcc functions.o client.c -o client 

remove: client server
	rm client
	rm server

