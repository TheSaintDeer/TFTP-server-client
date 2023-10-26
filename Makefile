CC=g++


all: client server

client: clean
	$(CC) src/tftp-client.cpp src/packet.cpp -o bin/tftp-client

server: clean
	$(CC) src/tftp-server.cpp src/packet.cpp -o bin/tftp-server

run-client:
	./bin//tftp-client -h 0.0.0.0 -p 8080 -f folderServer/testfile.txt -t folderClient/testfile.txt

run-server:
	./bin//tftp-server -p 8080 serverDownload

clean:
	rm -rf bin/*