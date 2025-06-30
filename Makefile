all:
	cc -o disc_type disc_type.c `pkg-config --libs --cflags libbluray dvdread`

install:
	sudo cp -v disc_type /usr/local/bin
