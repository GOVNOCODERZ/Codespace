Лабораторная работа №6: Реализация алгоритмов для расчёта особых точек
(детектор Харриса) с использованием OpenMP и Intel oneTBB.

Состав исходных кодов:
- lab6_harris.cpp — реализация всех 5 шагов детектора Харриса (включая 4.1, 4.2, 4.3),
  сравнение классических контейнеров STL (std::vector) и потокобезопасного контейнера
  Intel oneTBB (oneapi::tbb::concurrent_vector), замеры для таблиц 6.1 и 6.2.
- BMPFileRW.h     — модуль чтения/записи 24-битных BMP-файлов.
- common_stats.h  — функции AvgTrustedIntervalAVG, AvgTrustedIntervalMed, bubbleSort.
- Makefile        — сборка в MSYS2 MinGW UCRT64 (GCC + OpenMP + Intel oneTBB).

Сборка (Windows, MSYS2 UCRT64):
  C:\msys64\ucrt64\bin\g++.exe -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra -Werror lab6_harris.cpp -ltbb12 -o lab6_harris.exe

Запуск:
  ./lab6_harris.exe         (полный режим: 12 запусков на серию + 100 запусков для ДИ)
  ./lab6_harris.exe --fast  (быстрый демонстрационный режим: 3 запуска на серию)
