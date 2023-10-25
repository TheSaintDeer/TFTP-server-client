CC=g++


all: client server

client: clean
	$(CC) tftp-client.cpp packet.cpp -o tftp-client

server: clean
	$(CC) tftp-server.cpp packet.cpp -o tftp-server

run-client:
	./tftp-client -h 0.0.0.0 -p 8080 -f folderServer/testfile.txt -t folderClient/testfile.txt

run-server:
	./tftp-server -p 8080 serverDownload

clean:
	rm -rf server client 