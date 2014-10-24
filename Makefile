
PREFIX=/usr/local

PROGS		= qrencode
CFLAGS		= -Wall -fPIC -shared
LDFLAGS		= -lqrencode -lpng


all: $(PROGS)
%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@.so $^
clean:
	$(RM) $(PROGS).so
