# A22
A22 for CS1C

This workspace contains implementations of four sorting algorithms and a small driver.

Build:

```
make
```

Run examples:

```
./sorts -m 1000000        # run merge sort on 1,000,000 elements
./sorts -a 10000          # run all sorts on 10,000 elements (selection/bubble/insertion may be slow for very large n)
```

Valgrind: to check for leaks, run valgrind against the built binary; the program uses only std::vector so there should be no leaks.

