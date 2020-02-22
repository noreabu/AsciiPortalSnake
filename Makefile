CFLAGS=-g -Wall
LDFLAGS=-lncurses

consolesnake:
	g++ $(CFLAGS) main.cpp -o consolesnake $(LDFLAGS)

clean:
	rm consolesnake

