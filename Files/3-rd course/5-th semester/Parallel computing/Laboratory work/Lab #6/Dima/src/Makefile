CXX = g++
CXXFLAGS = -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra -Werror
LDFLAGS = -ltbb12

all: lab6_harris.exe

lab6_harris.exe: lab6_harris.cpp BMPFileRW.h common_stats.h
	$(CXX) $(CXXFLAGS) lab6_harris.cpp $(LDFLAGS) -o lab6_harris.exe

clean:
	rm -f lab6_harris.exe output_*.bmp lab6_*.csv
