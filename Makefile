CPPFLAGS = -Wall -O3 -std=c++11 -lm -w -msse2 -mbmi
PROGRAMS = main

main: main.cpp ASketch.h ASketch_multi_filter.h params.h BOBHash32.h CMSketch.h
	g++ -o main main.cpp $(CPPFLAGS)

clean:
	rm -f *.o $(PROGRAMS)
