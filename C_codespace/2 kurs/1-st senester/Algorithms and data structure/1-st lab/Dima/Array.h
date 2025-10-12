#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <stdexcept>
#include <algorithm>

struct SortStatistics {
    long long comparisons = 0;
    long long swaps = 0;
};

template<typename T>
class Array {
private:
    T* data;
    int size;

    void heapify(int n, int i, SortStatistics& stats) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n && ++stats.comparisons && data[left] > data[largest]) {
            largest = left;
        }

        if (right < n && ++stats.comparisons && data[right] > data[largest]) {
            largest = right;
        }

        if (largest != i) {
            std::swap(data[i], data[largest]);
            stats.swaps++;
            heapify(n, largest, stats);
        }
    }

public:
    Array() : data(nullptr), size(0) {}

    Array(int size) : size(size) {
        if (size < 0) throw std::invalid_argument("Размер списка не может быть отрицательным");
        data = new T[size];
    }

    Array(const Array& other) : size(other.size) {
        data = new T[size];
        for (int i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }

    Array& operator=(const Array& other) {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = new T[size];
            for (int i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    ~Array() {
        delete[] data;
    }

    // Метод для доступа к элементам
    T& operator[](int index) {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Индекс за пределами диапазона");
        }
        return data[index];
    }

    void inputFromKeyboard() {
        std::cout << "Введите " << size << " элементов массива:\n";
        for (int i = 0; i < size; ++i) {
            std::cout << "Элемент [" << i << "]: ";
            std::cin >> data[i];
        }
    }

    void fillRandom(T min_val, T max_val) {
        std::random_device rd;
        std::mt19937 gen(rd());

        if constexpr (std::is_integral<T>::value) {
            std::uniform_int_distribution<T> dist(min_val, max_val);
            for (int i = 0; i < size; ++i) {
                data[i] = dist(gen);
            }
        } else {
            std::uniform_real_distribution<T> dist(min_val, max_val);
            for (int i = 0; i < size; ++i) {
                data[i] = dist(gen);
            }
        }
    }
    
    void fillSorted() {
        for (int i = 0; i < size; ++i) {
            data[i] = static_cast<T>(i);
        }
    }
    
    void fillReverseSorted() {
        for (int i = 0; i < size; ++i) {
            data[i] = static_cast<T>(size - 1 - i);
        }
    }

    void display() const {
        std::cout << "Массив (первые 20 элементов): ";
        for (int i = 0; i < std::min(size, 20); ++i) {
            std::cout << data[i] << " ";
        }
        if (size > 20) {
            std::cout << "...";
        }
        std::cout << std::endl;
    }

    void writeToFile(const std::string& filename, const Array<T>& sorted_arr) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
            return;
        }
        
        file << "Исходный массив:\n";
        for (int i = 0; i < this->size; ++i) {
            file << this->data[i] << " ";
        }

        file << "\n\nОтсортированный массив:\n";
        for (int i = 0; i < sorted_arr.size; ++i) {
            file << sorted_arr.data[i] << " ";
        }

        file.close();
        std::cout << "Массивы успешно записаны в файл " << filename << std::endl;
    }

    SortStatistics shellSort() {
        SortStatistics stats;
        for (int gap = size / 2; gap > 0; gap /= 2) {
            for (int i = gap; i < size; ++i) {
                T temp = data[i];
                int j;
                for (j = i; j >= gap && ++stats.comparisons && data[j - gap] > temp; j -= gap) {
                    data[j] = data[j - gap];
                    stats.swaps++;
                }
                data[j] = temp;
                if (j != i) {
                    stats.swaps++;
                }
            }
        }
        return stats;
    }

    SortStatistics heapSort() {
        SortStatistics stats;
        
        for (int i = size / 2 - 1; i >= 0; --i) {
            heapify(size, i, stats);
        }

        for (int i = size - 1; i > 0; --i) {
            std::swap(data[0], data[i]);
            stats.swaps++;
            heapify(i, 0, stats);
        }
        return stats;
    }
    
    int getSize() const { return size; }
};

#endif