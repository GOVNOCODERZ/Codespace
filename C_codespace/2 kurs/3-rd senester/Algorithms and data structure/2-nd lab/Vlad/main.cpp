#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <clocale>
#include <iomanip>
#include "direct_merge_sort.h"
#include "balanced_multiway_merge_sort.h" 
using namespace std;

// Перечисление для выбора режима генерации данных
enum DataGenerationMode {
    RANDOM, // Случайные значения
    SORTED, // Отсортированная последовательность
    REVERSE_SORTED, // Последовательность в обратном порядке
};

/// @brief Генерации файла с данными для сортировки
/// @param filename имя файла для записи
/// @param size количество элементов
/// @param min_val минимальное значение элемента
/// @param max_val максимальное значение элемента
/// @param mode режим генерации данных
void generateDataFile(const string& filename, int size, int min_val, int max_val, DataGenerationMode mode) {
    ofstream file(filename);
    if (!file) {
        cerr << "ERROR: Failed to create file for writing data." << endl;
        return;
    }

    vector<int> data(size);
    
    switch (mode) // Генерируем случайные данные в зависимости от режима
    {
    case RANDOM:
        {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> distrib(min_val, max_val);
            for (int i = 0; i < size; ++i) {
                data[i] = distrib(gen);
            }
            break;
        }
    case SORTED:
        {
            for (int i = 0; i < size; ++i) {
                data[i] = i + min_val;
            }
            break;            
        }        
    case REVERSE_SORTED:
        {
            for (int i = 0; i < size; ++i) {
                data[i] = i + min_val;
            }
            reverse(data.begin(), data.end());
            break;
        }
    default:
        break;
    }

    // Записываем данные в файл в строчку через пробел
    for (int i = 0; i < size; ++i) {
        file << data[i] << (i == size - 1 ? "" : " "); // Не добавляем пробел после последнего элемента
    }
    file.close();
}

/// @brief Выполнение сортировки в одиночном режиме
void singleSortMode() {
    int size, min_val, max_val, method;
    string filename = "data.txt";

    // Получаем параметры последовательности от пользователя
    cout << "Enter the size of the numeric sequence: ";
    cin >> size;
    cout << "Enter the minimum value of an element: ";
    cin >> min_val;
    cout << "Enter the maximum value of an element: ";
    cin >> max_val;

    // Генерируем файл со случайными данными
    generateDataFile(filename, size, min_val, max_val, RANDOM);
    cout << "File \"" << filename << "\" with random data has been generated." << endl;

    // Пользователь выбирает метод сортировки
    cout << "\nChoose the external sorting method:\n";
    cout << "1. Direct merge sort\n";
    cout << "2. Balanced multiway merge sort\n";
    cout << "Your choice: ";
    cin >> method;

    SortStats stats;
    if (method == 1) {
        // Выполняем прямую сортировку слиянием
        stats = directMergeSort(filename);
    } else if (method == 2) {
        // Выполняем многопутевую сортировку слиянием
        int m; // Количество путей для сортировки
        cout << "Enter the number of ways (m) for merging (e.g., 3): ";
        cin >> m;
        stats = balancedMultiwayMergeSort(filename, m);
    } else {
        cout << "ERROR: Invalid choice." << endl;
        return;
    }

    // Выводим результаты сортировки
    cout << "\nSorting results:\n";
    cout << "Execution time: " << stats.time_ms << " ms\n";
    cout << "Number of comparisons: " << stats.comparisons << "\n";
    cout << "Number of swaps (writes): " << stats.swaps << "\n";
    cout << "Sorted data is in the file \"" << filename << "\"." << endl;
}

/// @brief Выполнение сортировки в режима сбора статистики
void statisticsMode() {
    int data_mode_choice, sort_method_choice;
    int start_size, end_size, step;
    string output_filename;

    // Выбор режима генерации данных
    cout << "Choose the method of forming the sequence elements:\n";
    cout << "1. Random values\n";
    cout << "2. Sorted sequence\n";
    cout << "3. Values in reverse order\n";
    cout << "Your choice: ";
    cin >> data_mode_choice;

    // Преобразуем выбор пользователя в enum
    DataGenerationMode data_mode;
    switch (data_mode_choice) {
        case 1: data_mode = RANDOM; break;
        case 2: data_mode = SORTED; break;
        case 3: data_mode = REVERSE_SORTED; break;
        default: cout << "ERROR: Invalid choice.\n"; return;
    }

    // Получаем диапазон и шаг изменения размера данных
    cout << "Set the range of sequence size change:\n";
    cout << "Start size: ";
    cin >> start_size;
    cout << "End size: ";
    cin >> end_size;
    cout << "Step size: ";
    cin >> step;

    // Выбор метода сортировки
    cout << "\nChoose the external sorting method:\n";
    cout << "1. Direct merge sort\n";
    cout << "2. Balanced multiway merge sort\n";
    cout << "Your choice: ";
    cin >> sort_method_choice;
    
    int m;
    if(sort_method_choice == 2) {
        cout << "Enter the number of ways (m) for merging (e.g., 3): ";
        cin >> m;
    }

    // Получаем имя файла для сохранения статистики
    cout << "Enter the filename to save results (e.g., stats.csv): ";
    cin >> output_filename;

    // Открываем файл для записи статистики
    ofstream stats_file(output_filename);
    if (!stats_file) {
        cerr << "Failed to create statistics file." << endl;
        return;
    }

    // Записываем заголовок CSV файла
    stats_file << "Size Time_(ms)\n";
    // Устанавливаем формат вывода чисел
    stats_file << fixed << setprecision(0);
    cout << "\nStarting statistics collection...\n";

    // Цикл по различным размерам данных
    for (int size = start_size; size <= end_size; size += step) {
        cout << "Sorting for size " << size << "..." << endl;
        string temp_filename = "temp_data.txt";
        // Генерируем временный файл с данными в выбранном режиме
        generateDataFile(temp_filename, size, 0, size * 10, data_mode);

        SortStats stats;
        if (sort_method_choice == 1) {
            // Выполняем прямую сортировку слиянием
            stats = directMergeSort(temp_filename);
        } else if (sort_method_choice == 2) {
            // Выполняем многопутевую сортировку слиянием
            stats = balancedMultiwayMergeSort(temp_filename, m);
        } else {
            cout << "Invalid sorting method choice." << endl;
            return;
        }

        // Записываем результаты в файл статистики
        stats_file << size << " " << stats.time_ms << "\n";
    }

    stats_file.close();
    cout << "\nStatistics collection finished. Results saved to file \"" << output_filename << "\"." << endl;
}


int main() {
    int mode;
    do {
        cout << "\n===== Laboratory Word #2 =====\n";
        cout << "Goal: Comparative analysis of external memory sorting algorithmes efficiency.\n";
        cout << "Level 'Good': Direct merge sorting, balanced multiway merge sorting.\n\n";

        cout << "============= MENU =============\n";        
        cout << "1. Sort a single data file\n";
        cout << "2. Statistical data collection mode\n";
        cout << "3. Close the program\n";
        cout << "Choose execution mode: ";

        cin >> mode;

        switch (mode) {
            case 1:
                singleSortMode();
                break;
            case 2:
                statisticsMode();
                break;
            case 3:
                cout << "Exiting the program..." << endl;
                break;
            default:
                cout << "ERROR: Invalid choice." << endl;
        }

    } while (mode != 3);

    return 0;
}