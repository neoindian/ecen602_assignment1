all: server client

server:  
	g++ -pthread  -std=c++0x -o server Server.cpp -I .

client: 
	g++  -o client client.cpp -I .

clean:
	rm -rf *.o server client
