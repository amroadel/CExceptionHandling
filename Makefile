all: app

throw.o: test/throw.cpp test/throw.h
		g++ -c -o throw.o -O0 -ggdb test/throw.cpp

libsupcpp.o: libsupcpp.cpp
		g++ -c -o libsupcpp.o -O0 -ggdb libsupcpp.cpp

main.o: test/main.c
		gcc -c -o main.o -O0 -ggdb test/main.c

app: main.o throw.o libsupcpp.o
		gcc main.o throw.o libsupcpp.o -O0 -ggdb -o app

throw.gas: throw.cpp
		g++ -c throw.cpp -g -Wa,-adhls > throw.gas

throw.s: throw.cpp
		g++ -S throw.cpp

.PHONY: clean run
clean:
	rm -f main.o throw.o libsupcpp.o app

run: app
		./app

