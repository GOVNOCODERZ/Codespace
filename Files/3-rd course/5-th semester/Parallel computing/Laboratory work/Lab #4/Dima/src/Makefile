CXX      = g++
CXXFLAGS = -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra
LDFLAGS  = -ltbb12

all: task4_1.exe task4_2.exe task4_3.exe

task4_1.exe: task4_1_matrix.cpp common_stats.h
	$(CXX) $(CXXFLAGS) task4_1_matrix.cpp $(LDFLAGS) -o task4_1.exe

task4_2.exe: task4_2_median.cpp BMPFileRW.h common_stats.h
	$(CXX) $(CXXFLAGS) task4_2_median.cpp $(LDFLAGS) -o task4_2.exe

task4_3.exe: task4_3_texture.cpp BMPFileRW.h common_stats.h
	$(CXX) $(CXXFLAGS) task4_3_texture.cpp $(LDFLAGS) -o task4_3.exe

clean:
	rm -f task4_1.exe task4_2.exe task4_3.exe
