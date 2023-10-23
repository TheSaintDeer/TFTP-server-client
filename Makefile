CC=g++


all: client server

client: clean
	$(CC) tftp-client.cpp packet.cpp -o client

server: clean
	$(CC) tftp-server.cpp packet.cpp -o server

clean:
	rm -rf server client 