#CC=g++
CC=clang++
CFLAGS=-std=c++11 -Wall -pedantic -O2 -ggdb
LIBS=-lm

.PHONY: all test testA testB

all: bms1A bms1B


bms1A: bms1A.cpp
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ libsndfile.a

bms1B: bms1B.cpp
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ libsndfile.a

clean:
	rm -f *.o bms1A bms1B
run:
	./bms1A test/input.txt test/signal.wav
#	./bms1B test/out.txt > demod.txt

testA: 
	./bms1A in.txt
	mv in.wav out.wav
test:
	./bms1A in.txt
	mv in.wav out.wav
	./bms1B out.wav

