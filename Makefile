LIBNAME= qrencode
CFLAGS= -std=gnu99 -pedantic -Wall -Wextra -O2 -shared -fPIC

all: so

so:	$(LIBNAME).c
	$(CC) $(CFLAGS) $(LIBNAME).c  -lqrencode -lpng -o $(LIBNAME).so

clean:
	rm -f $(LIBNAME).so 

