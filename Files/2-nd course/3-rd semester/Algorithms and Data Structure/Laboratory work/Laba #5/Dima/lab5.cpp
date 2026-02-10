#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;

// Состояния ячейки таблицы
enum CellState {
    EMPTY,      // Пусто (никогда не было данных)
    OCCUPIED,   // Занято (хранится активный элемент)
    DELETED     // Удалено (данные были, но удалены; маркер для продолжения поиска)
};

// Структура ячейки хеш-таблицы
template <typename T>
struct HashEntry {
    T value;
    CellState state;

    HashEntry() : state(EMPTY), value(T()) {}
};

// Шаблонный класс Хеш-таблица
template <typename T>
class HashTable {
private:
    vector<HashEntry<T>> table; // Массив ячеек
    int capacity;               // Общий размер таблицы (m)
    int size;                   // Текущее количество элементов (n)

    // Вспомогательный метод: перевод строки в числовое значение
    // Суммирует ASCII коды всех символов строки
    unsigned long long stringToLong(const string& key) const {
        unsigned long long sum = 0;
        for (char c : key) {
            sum += static_cast<unsigned char>(c);
        }
        return sum;
    }

    int hash1(const string& key) const {
        unsigned long long num = stringToLong(key);
        unsigned long long squared = num * num;

        // Извлечение "середины". Делим на 10, чтобы отбросить последнюю цифру, 
        // и берем модуль по размеру таблицы. Это простая имитация выделения средних разрядов.
        unsigned long long middle = squared / 10;
        
        return (int)(middle % capacity);
    }

    // Используется для вычисления шага смещения при коллизии.
    // Должна возвращать значение > 0 и взаимно простое с capacity.
    int hash2(const string& key) const {
        unsigned long long num = stringToLong(key);
        // Формула: 1 + (k mod (m-1)) гарантирует результат от 1 до m-1
        return (int)(1 + (num % (capacity - 1)));
    }

public:
    // Конструктор
    HashTable(int cap = 41) : capacity(cap), size(0) {
        table.resize(capacity);
    }

    // Очистка таблицы
    void clear() {
        for (auto& entry : table) {
            entry.state = EMPTY;
            entry.value = T();
        }
        size = 0;
    }

    bool insert(const T& key) {
        if (size >= capacity) {
            cout << "Ошибка: Таблица полностью заполнена!" << endl;
            return false;
        }

        int h1 = hash1(key);
        int h2 = hash2(key);
        int index = h1;

        // Первый проход: проверяем, нет ли уже такого элемента (дубликат) и ищем место для вставки
        int firstDeletedIndex = -1; // Запоминаем первую встреченную удаленную ячейку

        for (int i = 0; i < capacity; ++i) {
            index = (h1 + i * h2) % capacity;

            if (table[index].state == OCCUPIED && table[index].value == key) {
                return false;
            }

            if (table[index].state == EMPTY) {
                if (firstDeletedIndex == -1) {
                    table[index].value = key;
                    table[index].state = OCCUPIED;
                    size++;
                    return true;
                } else {
                    break;
                }
            }

            if (table[index].state == DELETED && firstDeletedIndex == -1) {
                firstDeletedIndex = index; // Запомнили место, но ищем дальше (вдруг дубликат)
            }
        }
        // Вставляем либо в найденное удаленное место, либо (если не нашли DELETED) цикл бы прервался на EMPTY.
        if (firstDeletedIndex != -1) {
            table[firstDeletedIndex].value = key;
            table[firstDeletedIndex].state = OCCUPIED;
            size++;
            return true;
        } else if (table[index].state == EMPTY) { 
            // Это случай, когда цикл прервался по break (не было DELETED ранее)
            table[index].value = key;
            table[index].state = OCCUPIED;
            size++;
            return true;
        }

        return false;
    }

    bool remove(const T& key) {
        int h1 = hash1(key);
        int h2 = hash2(key);
        int index;

        for (int i = 0; i < capacity; ++i) {
            index = (h1 + i * h2) % capacity;

            if (table[index].state == EMPTY) {
                return false; // Цепочка оборвалась, элемента нет
            }

            if (table[index].state == OCCUPIED && table[index].value == key) {
                table[index].state = DELETED; // Помечаем как удаленное
                size--;
                return true;
            }
        }
        return false;
    }

    void search(const T& key) {
        int h1 = hash1(key);
        int h2 = hash2(key);
        int index;
        
        int comparisons = 0;
        int collisions = 0; // В данном контексте = число неудачных проб (i)

        bool found = false;

        cout << "\n--- Результаты поиска ---" << endl;
        cout << "Ключ: " << key << endl;
        cout << "Hash1 (нач. индекс): " << h1 << endl;
        cout << "Hash2 (шаг): " << h2 << endl;

        for (int i = 0; i < capacity; ++i) {
            index = (h1 + i * h2) % capacity;
            comparisons++; // Сравниваем состояние ячейки и значение

            if (table[index].state == EMPTY) {
                break; // Точно не найден
            }
            if (table[index].state == OCCUPIED) {
                if (table[index].value == key) {
                    found = true;
                    cout << "СТАТУС: НАЙДЕН по индексу " << index << endl;
                    break;
                } else {
                    collisions++; // Ячейка занята другим элементом
                }
            } else {
            }
        }
        if (!found) cout << "СТАТУС: НЕ НАЙДЕН" << endl;
        cout << "Количество сравнений: " << comparisons << endl;
        cout << "Количество коллизий (проходов): " << collisions << endl;
        cout << "-------------------------" << endl;
    }

    // 7. СРЕДНЯЯ СТАТИСТИКА (ДЛЯ ОТЧЕТА)
    // Пробегает по всем элементам и считает, сколько в среднем нужно шагов, чтобы их найти.
    void printAverageStats() {
        if (size == 0) {
            cout << "Таблица пуста, статистика недоступна." << endl;
            return;
        }

        long long totalComparisons = 0;
        long long totalCollisions = 0;

        for (int i = 0; i < capacity; ++i) {
            if (table[i].state == OCCUPIED) {
                // Имитируем поиск этого элемента
                T key = table[i].value;
                int h1 = hash1(key);
                int h2 = hash2(key);
                int idx;

                for (int j = 0; j < capacity; ++j) {
                    idx = (h1 + j * h2) % capacity;
                    totalComparisons++;
                    
                    if (table[idx].state == OCCUPIED && table[idx].value == key) {
                        // Нашли. Число коллизий для этого элемента = j
                        totalCollisions += j;
                        break;
                    }
                }
            }
        }

        cout << "\n=== СТАТИСТИКА ДЛЯ ОТЧЕТА ===" << endl;
        cout << "Размер таблицы (m): " << capacity << endl;
        cout << "Элементов (n): " << size << endl;
        cout << "Коэффициент заполнения: " << fixed << setprecision(2) << (double)size / capacity * 100 << "%" << endl;
        cout << "Среднее число сравнений: " << (double)totalComparisons / size << endl;
        cout << "Среднее число коллизий:  " << (double)totalCollisions / size << endl;
        cout << "=============================" << endl;
    }

    // 1. Загрузка из файла
    void loadFromFile(string filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Ошибка: Не удалось открыть файл " << filename << endl;
            return;
        }
        
        clear(); // Очищаем перед новой загрузкой
        string word;
        while (file >> word) {
            // Ограничение длины по заданию (32 символа)
            if (word.length() > 32) word = word.substr(0, 32);
            insert(word);
        }
        file.close();
        cout << "Данные загружены из файла." << endl;
    }

    // 2. Выгрузка в файл
    void saveToFile(string filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "Ошибка: Не удалось создать файл " << filename << endl;
            return;
        }

        for (int i = 0; i < capacity; ++i) {
            if (table[i].state == OCCUPIED) {
                file << table[i].value << endl;
            }
        }
        file.close();
        cout << "Данные сохранены в файл." << endl;
    }

    // 6. Вывод таблицы на экран
    void printTable() {
        cout << "\n--- Содержимое Хеш-таблицы ---" << endl;
        cout << "Ind | St | Value" << endl;
        cout << "----|----|----------------" << endl;
        for (int i = 0; i < capacity; ++i) {
            if (table[i].state == OCCUPIED) {
                cout << setw(3) << i << " | OK | " << table[i].value << endl;
            } else if (table[i].state == DELETED) {
                cout << setw(3) << i << " | DEL| " << "---" << endl;
            }
            // Пустые строки (EMPTY) можно не выводить для компактности
        }
        cout << "------------------------------" << endl;
    }
};

void showMenu() {
    cout << "\nЛАБОРАТОРНАЯ РАБОТА №5 (Вариант 13)\n";
    cout << "1. Загрузить из файла\n";
    cout << "2. Сохранить в файл\n";
    cout << "3. Поиск элемента\n";
    cout << "4. Добавить элемент\n";
    cout << "5. Удалить элемент\n";
    cout << "6. Показать таблицу\n";
    cout << "7. ОБЩАЯ СТАТИСТИКА (для отчета)\n";
    cout << "0. Выход\n";
    cout << "> ";
}

int main() {
    // Создаем таблицу. Размер 41 (простое число) хорош для тестов.
    // Можно поменять на 7 или 11, чтобы быстрее вызвать коллизии.
    HashTable<string> ht(41); 
    
    int choice;
    string buffer;

    while (true) {
        showMenu();
        if (!(cin >> choice)) { // Защита от ввода букв вместо цифр
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        switch (choice) {
        case 1:
            cout << "Введите имя файла (напр. input.txt): ";
            cin >> buffer;
            ht.loadFromFile(buffer);
            break;
        case 2:
            cout << "Введите имя файла для сохранения: ";
            cin >> buffer;
            ht.saveToFile(buffer);
            break;
        case 3:
            cout << "Введите слово для поиска: ";
            cin >> buffer;
            ht.search(buffer);
            break;
        case 4:
            cout << "Введите слово для добавления: ";
            cin >> buffer;
            if (ht.insert(buffer)) cout << "Успешно добавлено.\n";
            else cout << "Не добавлено (дубликат или нет места).\n";
            break;
        case 5:
            cout << "Введите слово для удаления: ";
            cin >> buffer;
            if (ht.remove(buffer)) cout << "Успешно удалено.\n";
            else cout << "Элемент не найден.\n";
            break;
        case 6:
            ht.printTable();
            break;
        case 7:
            ht.printAverageStats();
            break;
        case 0:
            cout << "Выход.\n";
            return 0;
        default:
            cout << "Неверный выбор.\n";
        }
    }
}