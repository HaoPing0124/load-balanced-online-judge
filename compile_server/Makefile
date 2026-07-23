compile_server:compile_server.cc
	g++ -o $@ $^ -std=c++11 -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm -f compile_server
	rm -f temp/*.exe
	rm -f temp/*.stdin
	rm -f temp/*.stdout
	rm -f temp/*.stderr
	rm -f temp/*.compile_error