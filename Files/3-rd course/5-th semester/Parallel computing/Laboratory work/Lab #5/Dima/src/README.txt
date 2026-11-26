Лабораторная работа №5: Алгоритмы свертки в OpenMP и oneTBB
============================================================

Состав исходного кода:
- common_stats.h       — расчёт доверительного интервала (AvgTrustedIntervalAVG, bubbleSort, AvgTrustedIntervalMed)
- BMPFileRW.h          — модуль чтения/записи 24-битных BMP-файлов (каф. ИВТ СибГУ)
- lab5_convolution.cpp — Задания 5.1 и 5.2: фильтр Гаусса (классический 2D и быстрый 2x1D с транспонированием)
                         и среднеарифметический фильтр (классический 2D и быстрый рекурсивный) на OpenMP и oneTBB
- Makefile             — сценарий сборки для MSYS2 UCRT64 (g++ + OpenMP + oneTBB)

Сборка в терминале VS Code (MSYS2 UCRT64):
  g++ -std=c++17 -fopenmp -O3 -march=native -Wall -Wextra lab5_convolution.cpp -ltbb12 -o lab5_convolution.exe

Запуск:
  ./lab5_convolution.exe        — полное экспериментальное исследование (4 изображения, Ksize=5,9,13, 15 запусков)
  ./lab5_convolution.exe --fast — быстрый демонстрационный прогон (3 запуска) для защиты в аудитории
