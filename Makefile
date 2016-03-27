all:
	gcc -std=c11 -Wall -Wextra -pedantic -o webclient webclient.c 

clean:
	rm webclient
