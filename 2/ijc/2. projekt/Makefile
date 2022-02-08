CC=gcc
CFLAGS=-g -std=c99 -pedantic -Wall -Wextra -fPIC 

TARGETS = tail wordcount wordcount-dynamic libhtab.a libhtab.so
HTAB_SOURCES = htab_bucket_count.o htab_erease.o htab_for_each.o htab_hash_function.o htab_lookup_add.o htab_clear.o htab_find.o htab_free.o htab_init.o htab_size.o htab_move.o

all: $(TARGETS)

tail: tail.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

wordcount: wordcount.o io.o libhtab.a
	$(CC) $(CFLAGS) -static -o $@ wordcount.o io.o -L. -lhtab
wordcount-dynamic: wordcount.o io.o libhtab.so
	$(CC) $(CFLAGS) -o $@ wordcount.o io.o -L. -lhtab


%_indp.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@


libhtab.a: 	$(HTAB_SOURCES)
	ar rcs $@ $^
libhtab.so: $(HTAB_SOURCES)#$(patsubst %.o,%_indp.o,$(HTAB_SOURCES))
	$(CC) -shared $(CFLAGS) -o $@ $^

.PHONE: clean zip all set_library_path run-dynamic run-static

run-dynamic:
	LD_LIBRARY_PATH="." ./wordcount-dynamic

run-static:
	./wordcount

clean:
	rm -f *.o
	rm -f xharag02.zip
	rm -f $(TARGETS)

zip:
	zip xharag02.zip *.c *.h Makefile
