compile_server:compile_server.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f compile_server
	rm -f temp/*.exe
	rm -f temp/*.compile_error