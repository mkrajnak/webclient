all:
	gcc -Wall -Wextra -pedantic -o webclient webclient.c 

clean:
	rm webclient
