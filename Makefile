test = ./test/
build = ./build/
dir = ./
src = ./
inc = ./
unwind_objects = $(build)test-unwind.o $(build)test-unwind-pe.o $(build)test-unwind-eh.o 
libsupcpp_objects = $(build)libsupcpp.o
test_unwind_objects = $(build)unwind.o $(build)read_elf.o $(unwind_objects)
test_libsupcpp_objects = $(build)seppuku.o $(build)throw.o $(libsupcpp_objects)
aux = $(build)throw.gas $(build)throw.s
options = -ggdb

build: build_dir test_unwind

test_unwind: $(test_unwind_objects)
	gcc $(options) $(test_unwind_objects) -o $(build)test_unwind.out
	rm $(test_unwind_objects)

test_libsupcpp: $(test_libsupcpp_objects)
	gcc $(options) $(test_libsupcpp_objects) -o $(build)test_libsupcpp.out
	rm $(test_libsupcpp_objects)

test_unwind_all: $(test_unwind_objects)
	gcc $(options) $(test_unwind_objects) -o $(build)test_unwind.out

test_libsupcpp_all: $(test_libsupcpp_objects)
	gcc $(options) $(test_libsupcpp_objects) -o $(build)test_libsupcpp.out

# test_unwind
$(build)test-unwind.o: $(src)test-unwind.c $(inc)test-unwind.h
	gcc -c $(options) $(src)test-unwind.c -o $(build)test-unwind.o

$(build)test-unwind-pe.o: $(src)test-unwind-pe.c $(inc)test-unwind-pe.h
	gcc -c $(options) $(src)test-unwind-pe.c -o $(build)test-unwind-pe.o

$(build)test-unwind-eh.o: $(src)test-unwind-eh.c $(inc)test-unwind-eh.h
	gcc -c $(options) $(src)test-unwind-eh.c -o $(build)test-unwind-eh.o

$(build)unwind.o: $(test)unwind.c
	gcc -c $(options) $(test)unwind.c -o $(build)unwind.o

$(build)read_elf.o: $(test)read_elf.c
	gcc -c $(options) $(test)read_elf.c -o $(build)read_elf.o

# test_libsupcpp
$(build)libsupcpp.o: $(src)libsupcpp.cpp
	g++ -c $(options) $(src)libsupcpp.cpp -o $(build)libsupcpp.o

$(build)throw.o: $(test)throw.cpp $(test)throw.h
	g++ -c $(options) $(test)throw.cpp -o $(build)throw.o

$(build)seppuku.o: $(test)seppuku.c
	gcc -c $(options) $(test)seppuku.c -o $(build)seppuku.o

# aux
throw.gas: $(test)throw.cpp
	g++ -c $(test)throw.cpp -g -Wa,-adhls > $(build)throw.gas

throw.s: $(test)throw.cpp
	g++ -S $(test)throw.cpp -o $(build)throw.s

.PHONY: clean build_dir
clean:
	rm -f $(test_unwind_objects) $(test_libsupcpp_objects) $(aux) $(build)test_libsupcpp.out $(build)test_unwind.out

build_dir:
	mkdir -p build