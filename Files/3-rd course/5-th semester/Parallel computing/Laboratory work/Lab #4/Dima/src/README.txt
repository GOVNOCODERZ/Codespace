Лабораторная работа №4: Алгоритмы обработки массивов и их реализация с использованием oneTBB
============================================================================================

Состав исходного кода:
- common_stats.h      — расчёт доверительного интервала (AvgTrustedIntervalAVG, bubbleSort, AvgTrustedIntervalMed)
- BMPFileRW.h         — модуль чтения/записи 24-битных BMP-файлов (каф. ИВТ СибГУ)
- task4_1_matrix.cpp  — Задание 4.1: сложение, поэлементное умножение, сумма и Val_range матриц (OpenMP + oneTBB)
- task4_2_median.cpp  — Задание 4.2: медианная фильтрация 24-бит BMP (посл., OpenMP, Intel oneTBB)
- task4_3_texture.cpp — Задание 4.3: расчёт текстурных признаков M2, R, U, E и цветных карт (посл., OpenMP, oneTBB 1D/2D)
- Makefile            — сценарий сборки для MSYS2 UCRT64 (g++ + OpenMP + oneTBB)

Сборка в терминале VS Code (MSYS2 UCRT64):
  g++ -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra task4_1_matrix.cpp -ltbb12 -o task4_1.exe
  g++ -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra task4_2_median.cpp -ltbb12 -o task4_2.exe
  g++ -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra task4_3_texture.cpp -ltbb12 -o task4_3.exe
