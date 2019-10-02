all:gpio_control_app

OBJS = gpio_control_app.o

gpio_control_app:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f gpio_control_app *.o
