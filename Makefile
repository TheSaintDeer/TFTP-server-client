CC=g++


all: client server

client: clean
	$(CC) tftp-client.cpp -o client

server: clean
	$(CC) tftp-server.cpp -o server

clean:
	rm -rf server client 