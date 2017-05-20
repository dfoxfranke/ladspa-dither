PREFIX ?= /usr/local
DESTDIR ?= $(PREFIX)

SOTARGET=dither_5341.so

dither.so: dither.c
	$(CC) -o dither_5341.so -Wall -Wconversion -O3 -ffast-math -ftree-vectorize -funroll-loops $(CPPFLAGS) $(CFLAGS) -fPIC -shared dither.c $(LDFLAGS) -lc -lm

clean:
	$(RM) dither_5341.so

all: dither_5341.so
install: all
	install -D dither_5341.so $(DESTDIR)/lib/ladspa
