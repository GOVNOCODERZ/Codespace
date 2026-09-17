// Task1_1_HelloWorld.cpp
// Задание 1.1: консольное приложение с поддержкой OpenMP.
// Две функции (cout и printf) создают X потоков, каждый из которых
// выводит приветственное сообщение от потока.

#include <iostream>
#include <cstdio>
#include <omp.h>
#include <windows.h>
#pragma execution_character_set( "utf-8" )

// Задание 1.1: вывод через std::cout
void HelloWorld_cout(int numThreads)
{
    std::cout << "\n[cout version]" << std::endl;

#pragma omp parallel num_threads(numThreads)
    {
        int id = omp_get_thread_num();
        int total = omp_get_num_threads();
        std::cout << "Hello World! From thread #" << id << " out of " << total << " threads" << std::endl;
    }
}

// Задание 1.1: вывод через printf
void HelloWorld_printf(int numThreads)
{
    printf("\n[printf version]\n");

#pragma omp parallel num_threads(numThreads)
    {
        int id = omp_get_thread_num();
        int total = omp_get_num_threads();
        printf("Hello World! From thread #%d out of %d threads\n", id, total);
    }
}

int main()
{
    // Для корректного вывода utf-8 в консоли VS 2019/2022
    SetConsoleOutputCP(65001);

    // Вывод максимально доступного в системе количества потоков
    int maxThreads = omp_get_max_threads();
    std::cout << "Maximum available threads in the system: " << maxThreads << std::endl;

    // Запрос у пользователя количества потоков X
    // (X может превышать maxThreads — OpenMP допускает oversubscription)
    int X = 0;
    std::cout << "Enter the number of threads to create (X): ";
    std::cin >> X;

    if (X <= 0)
    {
        std::cout << "Invalid value, using X = 1" << std::endl;
        X = 1;
    }

    HelloWorld_cout(X);
    HelloWorld_printf(X);

    return 0;
}