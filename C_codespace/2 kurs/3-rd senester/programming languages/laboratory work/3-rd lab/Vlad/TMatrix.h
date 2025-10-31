#ifndef TMATRIX_H
#define TMATRIX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
using namespace std;

template<typename T>
class TMatrix {
private:
    vector<vector<T>> data;
    int rows;
    int cols;

public:
    // Конструктор по умолчанию
    TMatrix();

    // Конструктор с размерами
    TMatrix(int r, int c);

    // Конструктор копирования
    TMatrix(const TMatrix& other);

    // Деструктор
    ~TMatrix();

    // Оператор присваивания
    TMatrix& operator=(const TMatrix& other);

    // Оператор сравнения
    bool operator==(const TMatrix& other) const;

    // Геттеры
    int getRows() const { return rows; }
    int getCols() const { return cols; }

    // Методы ввода/вывода
    void input();
    void output() const;

    // Перегрузка операторов ввода/вывода
    friend istream& operator>>(istream& is, TMatrix& m) {
        m.input();
        return is;
    }

    friend ostream& operator<<(ostream& os, const TMatrix& m) {
        m.output();
        return os;
    }

    // Методы по варианту
    int countPositiveBelowRowMean() const;
    T sumOfNegativeElements() const;

    // Метод генерации случайных значений
    void GenerateRandomValues(T min_val, T max_val);

    // Модификация элемента (ввод с консоли)
    void modifyElement(int row, int col);

    // Метод генерации случайного значения для элемента
    void SetRandomValue(int row, int col, T min_val, T max_val);

    // Загрузка из файла
    void loadFromFile(const string& filename);

    // Сохранение в файл
    void saveToFile(const string& filename) const;
};

#endif