server:
	g++ --std=c++11 server.cpp -o server.out
run_server: server
	./server.out 5001
client:
	g++ --std=c++11 client.cpp -o client.out
both:server client
	rm -rf ./server_storage/*
	cp client.out ./clientTest
	cp client.out ./clientTest2
	cp client.out ./clientTest3
	cp client.out ./clientTest4
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
