CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
LC_ALL=cs_CZ.utf8
LDLIBS=-lpthread

TARGETS = proj2 elf santa reindeer

all: $(TARGETS)
	
.PHONE: clean zip

clean:
	rm -f *.o
	rm -f xharag02.zip
	rm -f $(TARGETS)

zip:
	zip xharag02.zip *.c *.h Makefile
