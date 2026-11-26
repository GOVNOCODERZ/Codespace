CXX      = g++
CXXFLAGS = -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra

all: task3_1.exe task3_2.exe

task3_1.exe: task3_1_sort.cpp common_stats.h
	$(CXX) $(CXXFLAGS) task3_1_sort.cpp -o task3_1.exe

task3_2.exe: task3_2_median.cpp BMPFileRW.h common_stats.h
	$(CXX) $(CXXFLAGS) task3_2_median.cpp -o task3_2.exe

clean:
	rm -f task3_1.exe task3_2.exe
