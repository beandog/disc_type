all:
	gcc -o disc_type `pkg-config --libs --cflags libbluray dvdread` disc_type.c

install:
	doas cp -v disc_type /usr/local/bin
