all:
	clang++ -std=c++11 -Wall -Wextra -o webclient webclient.cpp 

clean:
	rm webclient
