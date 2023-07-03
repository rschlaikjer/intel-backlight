PROGNAME := backlight

all: backlight

clean:
	rm ${PROGNAME}

backlight:
	cc $(CFLAGS) -Wall -Wextra -Werror -o ${PROGNAME} backlight.c

install: backlight
	install -o root -g root -m 775 ${PROGNAME} /usr/local/bin/
	chmod +s /usr/local/bin/${PROGNAME}
