default: server.out client.out
client.out: client.cpp
						clang++ client.cpp -o client.out -lpthread 
server.out: server.cpp
						clang++ server.cpp -pthread -o server.out -lpthread 
