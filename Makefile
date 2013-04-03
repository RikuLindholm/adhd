CC=gcc
CFLAGS=-lgcrypt
EXECUTABLE=main
 
all:
	$(CC) $(CFLAGS) sha1.c main.c -o $(EXECUTABLE)

clean:
	rm -rf *.o $(EXECUTABLE)