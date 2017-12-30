all: main

main: server.cpp client.cpp
	g++ -o client client.cpp -l pthread
	g++ -o server server.cpp -l pthread
