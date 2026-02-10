#ifndef TFIGUREARRAY_H
#define TFIGUREARRAY_H

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include "FigureExceptions.h"

template <typename T>
class TFigureArray {
private:
    T* data;
    int size;

public:
    // Конструктор по умолчанию
    TFigureArray(int n = 0) : size(n) {
        if (n < 0) throw std::invalid_argument("Размер не может быть отрицательным");
        data = (size > 0) ? new T[size] : nullptr;
    }

    // Конструктор копирования (Базовая + Строгая гарантия)
    TFigureArray(const TFigureArray& other) : size(other.size) {
        data = (size > 0) ? new T[size] : nullptr;
        try {
            for (int i = 0; i < size; ++i) data[i] = other.data[i];
        } catch (...) {
            delete[] data;
            throw;
        }
    }

    // Конструктор перемещения (Move constructor) - noexcept для безопасности
    TFigureArray(TFigureArray&& other) noexcept : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
    }

    ~TFigureArray() { delete[] data; }

    // Оператор копирующего присваивания (Strong Exception Safety через Copy-and-Swap)
    TFigureArray& operator=(const TFigureArray& other) {
        if (this != &other) {
            TFigureArray temp(other);
            std::swap(data, temp.data);
            std::swap(size, temp.size);
        }
        return *this;
    }

    // Оператор перемещающего присваивания
    TFigureArray& operator=(TFigureArray&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }

    // Безопасный доступ (Пункт 3 - проверка границ)
    T& operator[](int index) {
        if (index < 0 || index >= size) 
            throw std::out_of_range("Индекс находится за предела");
        return data[index];
    }

    int getCount() const { return size; }

    void GenerateRandomValues() {
        for (int i = 0; i < size; ++i) data[i].setRandomValue();
    }

    // Метод Шелла (Действие 1 варианта)
    void shellSort() {
        for (int gap = size / 2; gap > 0; gap /= 2) {
            for (int i = gap; i < size; i++) {
                T temp = data[i];
                int j;
                for (j = i; j >= gap && data[j - gap] > temp; j -= gap) {
                    data[j] = data[j - gap];
                }
                data[j] = temp;
            }
        }
    }

    // Поиск выше среднего (Действие 2 варианта)
    void findAboveAverage() const {
        if (size == 0) return;
        double sum = 0;
        for (int i = 0; i < size; ++i) sum += data[i].getArea();
        double avg = sum / size;
        std::cout << "Средняя площадь: " << avg << "\n";
        for (int i = 0; i < size; ++i) {
            if (data[i].getArea() > avg) std::cout << "  " << data[i] << "\n";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const TFigureArray<T>& arr) {
        for (int i = 0; i < arr.size; ++i) {
            os << arr.data[i] << ( (&os == &std::cout) ? "\n" : " " );
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, TFigureArray<T>& arr) {
        for (int i = 0; i < arr.size; ++i) is >> arr.data[i];
        return is;
    }
};
#endif