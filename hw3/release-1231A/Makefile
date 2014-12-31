all:
	g++ --std=c++11 server.cpp -o server
	g++ --std=c++11 client.cpp -o client
server:
	g++ --std=c++11 server.cpp -o server
run_server: server
	./server.out 5001
client:
	g++ --std=c++11 client.cpp -o client
both:all
	rm -rf ./server_storage/*
	cp client ./clientTest
	cp client ./clientTest2
	cp client ./clientTest3
	cp client ./clientTest4
file:
	rm -rf ./clientTest2/*
	rm -rf ./clientTest4/*
	rm -rf ./clientTest/K*
	rm -rf ./clientTest3/K*
	cp L* ./clientTest
	cp L* ./clientTest3
	cp K* ./clientTest2
	cp K* ./clientTest4
all_test:file both
