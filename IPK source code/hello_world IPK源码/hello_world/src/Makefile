all: hello_world
hello_world: hello_world.o
	$(CC) $(LDFLAGS) hello_world.o -o hello_world
hello_world.o: hello_world.c
	$(CC) $(CFLAGS) -c hello_world.c
clean:
	rm *.o hello_world

