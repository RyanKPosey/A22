CFLAGS = -Wall -Werror -Wpedantic -std=c++20 -pthread -O2 -g
CC = g++

OBJECTS = main.o

run-tests: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.cpp bubbleSort.h insertionSort.h merge.h sysmem.h live_monitor.h

clean:
	rm -f run-tests *.o *~
