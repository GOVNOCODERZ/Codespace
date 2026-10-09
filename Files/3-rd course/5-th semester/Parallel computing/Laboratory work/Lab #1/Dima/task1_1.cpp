#include <iostream>
#include <cstdio>
#include <omp.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Вывод приветствия через cout
void hello_cout(int x) {
    #pragma omp parallel num_threads(x)
    {
        int id = omp_get_thread_num();
        int total = omp_get_num_threads();
        std::cout << "Hello World! От потока №" << id << " из " << total << " потоков" << std::endl;
    }
}

// Вывод приветствия через printf
void hello_printf(int x) {
    #pragma omp parallel num_threads(x)
    {
        int id = omp_get_thread_num();
        int total = omp_get_num_threads();
        printf("Hello World! От потока №%d из %d потоков\n", id, total);
    }
}

int main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "Максимально доступное количество потоков: "
              << omp_get_max_threads() << std::endl;

    int x;
    std::cout << "Введите количество потоков X: ";
    std::cin >> x;

    std::cout << "\n--- Вывод через cout ---\n";
    hello_cout(x);

    std::cout << "\n--- Вывод через printf ---\n";
    hello_printf(x);

    return 0;
}
