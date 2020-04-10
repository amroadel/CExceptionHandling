TESTDIR = ./
BUILDDIR = ../build/
DIR = ../
TARGET = app
OBJECTS = $(BUILDDIR)main.o $(BUILDDIR)throw.o $(BUILDDIR)libsupcpp.o $(BUILDDIR)unwind-test.o #add this when you want to test unwind-test
AUX = $(BUILDDIR)throw.gas $(BUILDDIR)throw.s
OPTIONS = -O0 -ggdb

build: $(TARGET)
	rm -f $(OBJECTS)

all: $(TARGET)

$(BUILDDIR)unwind-test.o: $(DIR)unwind-test.c $(DIR)unwind-test.h
	gcc -c $(OPTIONS) $(DIR)unwind-test.c -o $(BUILDDIR)unwind-test.o

$(BUILDDIR)libsupcpp.o: $(DIR)libsupcpp.cpp
	g++ -c $(OPTIONS) $(DIR)libsupcpp.cpp -o $(BUILDDIR)libsupcpp.o

$(BUILDDIR)throw.o: $(TESTDIR)throw.cpp $(TESTDIR)throw.h
	g++ -c $(OPTIONS) $(TESTDIR)throw.cpp -o $(BUILDDIR)throw.o

$(BUILDDIR)main.o: $(TESTDIR)main.c
	gcc -c $(OPTIONS) $(TESTDIR)main.c -o $(BUILDDIR)main.o

$(TARGET): $(OBJECTS)
	gcc $(OPTIONS) $(OBJECTS) -o $(BUILDDIR)$(TARGET)

throw.gas: $(TESTDIR)throw.cpp
	g++ -c $(TESTDIR)throw.cpp -g -Wa,-adhls > $(BUILDDIR)throw.gas

throw.s: $(TESTDIR)throw.cpp
	g++ -S $(TESTDIR)throw.cpp -o $(BUILDDIR)throw.s

.PHONY: clean run debug
clean:
	rm -f $(OBJECTS) $(AUX) $(BUILDDIR)$(TARGET)

run:
	$(BUILDDIR)$(TARGET)

debug:
	gdb $(BUILDDIR)$(TARGET)