all:
	g++ -Wall -Wextra -Werror -o webclient webclient.cpp 

clean:
	rm webclient
