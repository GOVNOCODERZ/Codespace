CXX      = g++
CXXFLAGS = -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra
LDFLAGS  = -ltbb12

all: lab5_convolution.exe

lab5_convolution.exe: lab5_convolution.cpp BMPFileRW.h common_stats.h
	$(CXX) $(CXXFLAGS) lab5_convolution.cpp $(LDFLAGS) -o lab5_convolution.exe

clean:
	rm -f lab5_convolution.exe
