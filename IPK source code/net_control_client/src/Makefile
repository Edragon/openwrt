all:net_control_client

OBJS = net_control_client.o

net_control_client:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lpthread

clean:
	rm -f net_control_client *.o