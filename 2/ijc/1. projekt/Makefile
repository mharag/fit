CC=gcc
CFLAGS=-g -std=c11 -pedantic -Wall -Wextra -O2 
#CFLAGS+=-m32
#CFLAGS += -fsanitize=address
LC_ALL=cs_CZ.utf8
LDLIBS=-lm

STEG_D_DEPS = steg-decode.o ppm.o error.o eratosthenes.o bitset.o
PRIMES_DEPS = primes.o error.o eratosthenes.o bitset.o

TARGETS = steg-decode primes primes-i

all: $(TARGETS)

steg-decode: $(STEG_D_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
primes: $(PRIMES_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
primes-i: $(patsubst %.o, %-inline.o, $(PRIMES_DEPS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%-inline.o: %.c %.o
	$(CC) $(CFLAGS) -DUSE_INLINE -o $@ -c $< $(LDLIBS)	

ppm.o: ppm.c ppm.h error.h bitset.h eratosthenes.h
bitset.o: bitset.c bitset.h
primes.o: primes.c bitset.h eratosthenes.h
steg-decode.o: steg-decode.c ppm.h error.h bitset.h eratosthenes.h
eratosthenes.o: eratosthenes.c bitset.h


.PHONE: clean run zip

run: primes primes-i 
	./primes
	./primes-i
clean:
	rm -f *.o
	rm -f xharag02.zip
	rm -f $(TARGETS)

zip:
	zip xharag02.zip *.c *.h Makefile
