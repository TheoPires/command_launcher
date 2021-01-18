CC = gcc -o -g -lm -lrt
CFLAGS = -std=c11 -Wall -Wconversion -Werror -Wextra -Wpedantic -pthread -D_XOPEN_SOURCE=500 -I file_src/ -I thread_pool/
LDFLAGS = -lrt
demon_objects = demon_src/demon.o file_src/file.o thread_pool/thread_pool.o
client_objects = client_src/client.o file_src/file.o

all: demon client

demon: $(demon_objects)
	$(CC) $(demon_objects) $(CFLAGS) -o demon $(LDFLAGS)

client: $(client_objects)
	$(CC) $(client_objects) $(CFLAGS) -o client $(LDFLAGS)

clean:
	rm -rf $(demon_objects) $(client_objects) demon client

client.o: client.c client.h
demon.o: demon.c demon.h
file.o: file.c file.h
thread_pool.o: thread_pool.c thread_pool.h

