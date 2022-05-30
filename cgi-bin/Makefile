CC = gcc

#Release
CFLAGS = -O2 -Wall 
#Debug
#CFLAGS = -g -Wall -I ..

all: adder 

adder: adder.c
	$(CC) $(CFLAGS) -o adder adder.c sha1.c


clean:
	rm -f adder *~
