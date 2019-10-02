all:net_control_server

OBJS = net_control_server.o

net_control_server:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lpthread

clean:
	rm -f net_control_server *.o