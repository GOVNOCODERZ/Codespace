#ifndef HASHTABLE_H
#define HASHTABLE_H

/// Максимальная длина входящей строки по заданию
#define MAX_STRING_LENGTH 32

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

/// @brief Состояния таблицы
enum CellState {
    EMPTY, // Ячейка пуста и никогда не использовалась
    OCCUPIED, // Ячейка занята элементом
    DELETED, // Ячейка была занята, но элемент удалён
};

/// @brief Ячейка хэш-таблицы
/// @tparam T тип данных
template <typename T>
struct HashCell {
    T value; // Значение/ключ
    CellState state; // Состояние ячейки

    // Конструктор по умолчанию (пустая ячейка)
    HashCell() : state(EMPTY), value(T()) {}
};

/// @brief Хеш-таблица с квадратичным пробированием
/// @tparam T тип данных
template <typename T>
class HashTable {
private:
    vector<HashCell<T>> table; // Массив ячеек таблицы
    int size; // Общая ёмкость таблицы (размер массива)
    int cells_count; // Количество занятых элементов

    /// @brief Преобразование строки в число
    /// @param key входящая строка
    /// @return строку как вещественное число
    unsigned long long stringToLong(const string& key) const {
        unsigned long long sum = 0;
        for (char c : key) { // Проходим по каждому символу строки
            sum += static_cast<unsigned char>(c);
        }
        return sum;
    }

    /// @brief Хеш-функция методом деления
    /// @param key значение для определения индекса
    /// @return Индекс элемента
    int hash_function(const string& key) const {
        unsigned long long numeric_key = stringToLong(key); // Вычисляем числовое значение ключа
        return static_cast<int>(numeric_key % size); // Вычисляем индекс: остаток от деления на размер таблицы
    }

public:

    /// @brief Конструктор класса
    /// @param cap начальный размер таблицы
    HashTable(int cap) : size(cap), cells_count(0) {
        table.resize(size); // Расширяем вектор до нужного размера
    }

    /// @brief Очистка таблицы. Сбрасывает до изначального состояния
    void clear() {
        for (auto& entry : table) {
            entry.state = EMPTY;
            entry.value = T();
        }
        cells_count = 0;
    }

    /// @brief Вставка элемента
    /// @param key значение для вставки
    /// @return результат вставки как `boolean`
    bool insert(const T& key) {
        if (cells_count >= size) {
            cout << "Error: Table is full!" << endl;
            return false;
        }

        int default_index = hash_function(key); // Вычисляем начальный индекс

        int i = 0; // Счётчик шагов пробирования
        int index = default_index; // Текущий индекс элемента
        int firstDeletedIndex = -1; // Индекс первой найденной ячейки со статусом DELETED

        while (table[index].state != EMPTY) { // Ищем место для вставки
            if (table[index].state == OCCUPIED && table[index].value == key) {
                return false; // Дубликат найден, вставка не требуется
            }

            if (table[index].state == DELETED && firstDeletedIndex == -1) {
                firstDeletedIndex = index; // Запоминаем индекс
            }
            i++;
            index = (default_index + i * i) % size; // Квадратичное пробирование

            if (i >= size) { // Защита от бесконечного цикла
                break;
            }
        }

        if (firstDeletedIndex != -1) { // Была найдена удалённая ячейка
            table[firstDeletedIndex].value = key;
            table[firstDeletedIndex].state = OCCUPIED;
        }
        else if (table[index].state == EMPTY) { // Была найдена пустая ячейка
            table[index].value = key;
            table[index].state = OCCUPIED;
        }
        else {
            cout << "Error: Cannot find space!" << endl;
            return false;
        }
        cells_count++;
        return true;
    }

    /// @brief Удаление элемента
    /// @param key значение для удаления
    /// @return результат удаления как `boolean`
    bool remove(const T& key) {
        int default_index = hash_function(key); // Вычисляем начальный индекс
        int i = 0; // Счётчик шагов
        int index = default_index; // Текущий индекс для поиска

        while (table[index].state != EMPTY) { // Найдена пустая ячейка с таким значением => конец цепочки поиска
            if (table[index].state == OCCUPIED && table[index].value == key) { // Найдена нужная ячейка
                table[index].state = DELETED;
                cells_count--;
                return true;
            }
            i++;
            index = (default_index + i * i) % size; // Квадратичное пробирование

            if (i >= size) { // Защита от бесконечного цикла
                break;
            }
        }
        return false;
    }

    /// @brief Поиск элемента
    /// @param key значение для удаления
    void search(const T& key) {
        int default_index = hash_function(key); // Вычисляем начальный индекс
        int i = 0; // Счётчик шагов
        int index = default_index; // Текущий индекс для поиска

        int comparisons = 0; // Счётчик сравнений
        int collisions = 0; // Счётчик коллизий (занятые, но не подходящие ячейки)

        cout << "\n--- Search Results ---" << endl;
        cout << "Key: " << key << endl;
        cout << "hash_function: " << default_index << endl;

        while (table[index].state != EMPTY) { // Цикл поиска элемента
            comparisons++;

            if (table[index].state == OCCUPIED) {
                if (table[index].value == key) {
                    cout << "STATUS: FOUND at index " << index << endl;
                    break;
                }
                else {
                    collisions++;
                }
            }

            i++;
            index = (default_index + i * i) % size; // Квадратичное пробирование

            if (i >= size) { // Защита от бесконечного цикла
                break;
            }
        }

        if (table[index].state == EMPTY || i >= size) {
            cout << "STATUS: NOT FOUND" << endl;
        }
        cout << "Comparisons: " << comparisons << ", Collisions: " << collisions << endl;
        cout << "-------------------------" << endl;
    }

    /// @brief Вывод статистики. Считает, сколько нужно сделать шагов чтобы найти элемент
    void printAverageStats() {
        if (cells_count == 0) {
            cout << "Table is empty, stats unavailable." << endl;
            return;
        }

        long long totalComparisons = 0; // Всего сравнений
        long long totalCollisions = 0;  // Всего коллизий

        for (int i = 0; i < size; ++i) { // Проход по всей таблице
            if (table[i].state == OCCUPIED) {
                T key = table[i].value;
                int default_index = hash_function(key); // начальный индекс
                int steps_count = 0; // Счётчик шагов для элемента
                int current_index = default_index; // Текущий индекс элемента


                while (true) { // Имитируем процесс поиска элемента
                    totalComparisons++;

                    if (table[current_index].state == OCCUPIED && table[current_index].value == key) {
                        totalCollisions += steps_count; // Ячейка найдена
                        break;
                    }
                    steps_count++;
                    current_index = (default_index + steps_count * steps_count) % size; // Квадратичное пробирование

                    // Защита от бесконечного цикла
                    if (steps_count >= size) {
                        break;
                    }
                }
            }
        }

        // Выводим итоговую статистику
        cout << "\n=== STATISTICS FOR REPORT ===" << endl;
        cout << "Size: " << size << ", Elements: " << cells_count << endl;
        cout << "Load factor: " << fixed << setprecision(2) << (double)cells_count / size * 100 << "%" << endl;
        cout << "Avg comparisons: " << (double)totalComparisons / cells_count << endl;
        cout << "Avg collisions:  " << (double)totalCollisions / cells_count << endl;
        cout << "=============================" << endl;
    }

    /// @brief Загрузка данных из файла
    /// @param filename имя файла
    void loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Cannot open file " << filename << endl;
            return;
        }

        clear();

        string word; // Переменная для хранения слова из файла

        while (file >> word) {
            if (word.length() > MAX_STRING_LENGTH) {
                word = word.substr(0, MAX_STRING_LENGTH);
            }
            insert(word);
        }
        file.close();
        cout << "Data loaded from file." << endl;
    }

    /// @brief Выгрузка данных в файл
    /// @param filename имя файла
    void saveToFile(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Cannot create file " << filename << endl;
            return;
        }

        for (int i = 0; i < size; ++i) {
            if (table[i].state == OCCUPIED) { // Берём только занятые ячейки
                file << table[i].value << endl;
            }
        }
        file.close();
        cout << "Data saved to file." << endl;
    }

    /// @brief Вывод таблицы в консоль
    void printTable() {
        cout << "\n--- Hash Table Contents ---" << endl;
        cout << "Index | State | Value" << endl; // Индекс | Состояние | Значение
        cout << "------|---------|----------------" << endl;

        for (int i = 0; i < size; ++i) {
            // Выводим ячейки, которые OCCUPIED
            if (table[i].state == OCCUPIED) {
                cout << i << " | OK | " << table[i].value << endl;
            }
            // Выводим ячейки, которые DELETED
            else if (table[i].state == DELETED) {
                cout << i << " | DEL | " << endl;
            }
        }
        cout << "------------------------------" << endl;
    }
};

#endif