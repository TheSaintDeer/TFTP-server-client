# Name: Ivan Golikov
# Login: xgolik00

CC=g++

all: client server

client: clean
	$(CC) src/tftp-client.cpp src/packet.cpp src/common.cpp -o bin/tftp-client

server: clean
	$(CC) src/tftp-server.cpp src/packet.cpp src/common.cpp -o bin/tftp-server

clean:
	rm -rf bin/*