default: server.out client.out
client.out: client.cpp
						g++ client.cpp -o client.out -lpthread
server.out: server.cpp
						g++ server.cpp -pthread -o server.out -lpthread
