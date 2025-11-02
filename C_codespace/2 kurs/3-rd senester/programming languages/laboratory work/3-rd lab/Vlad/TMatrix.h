#ifndef TMATRIX_H
#define TMATRIX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <limits>
#include <algorithm>
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

// Реализация методов шаблона в заголовочном файле

template<typename T>
TMatrix<T>::TMatrix() : rows(0), cols(0) {}

template<typename T>
TMatrix<T>::TMatrix(int r, int c) : rows(r), cols(c) {
    data.resize(rows, vector<T>(cols));
}

template<typename T>
TMatrix<T>::TMatrix(const TMatrix& other) : rows(other.rows), cols(other.cols) {
    data = other.data;
}

template<typename T>
TMatrix<T>::~TMatrix() {}

template<typename T>
TMatrix<T>& TMatrix<T>::operator=(const TMatrix& other) {
    if (this != &other) {
        rows = other.rows;
        cols = other.cols;
        data = other.data;
    }
    return *this;
}

template<typename T>
bool TMatrix<T>::operator==(const TMatrix& other) const {
    return (rows == other.rows && cols == other.cols && data == other.data);
}

template<typename T>
void TMatrix<T>::input() {
    cout << "Enter number of rows: ";
    cin >> rows;
    cout << "Enter number of columns: ";
    cin >> cols;

    data.resize(rows, vector<T>(cols));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << "Enter element [" << i << "][" << j << "]: ";
            cin >> data[i][j];
        }
    }
}

template<typename T>
void TMatrix<T>::output() const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << data[i][j] << " ";
        }
        cout << endl;
    }
}

template<typename T>
int TMatrix<T>::countPositiveBelowRowMean() const {
    int count = 0;
    for (int i = 0; i < rows; ++i) {
        T sum = T(0);
        for (int j = 0; j < cols; ++j) {
            sum = sum + data[i][j];
        }
        T mean = sum / static_cast<T>(cols);

        for (int j = 0; j < cols; ++j) {
            if (data[i][j] > T(0) && data[i][j] < mean) {
                count++;
            }
        }
    }
    return count;
}

template<typename T>
T TMatrix<T>::sumOfNegativeElements() const {
    T sum = T(0);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (data[i][j] < T(0)) {
                sum = sum + data[i][j];
            }
        }
    }
    return sum;
}

// --- НОВАЯ РЕАЛИЗАЦИЯ GenerateRandomValues ---
template<typename T>
void TMatrix<T>::GenerateRandomValues(T min_val, T max_val) {
    // Используем статическую переменную для инициализации генератора один раз
    static random_device rd;
    static mt19937 gen(rd());

    // Проверяем, поддерживает ли T операции сравнения и генерацию
    // Если T - пользовательский класс (например, Fraction), ему нужно уметь сравниваться и создаваться из double
    // uniform_real_distribution<double> dis(min_val.toDouble(), max_val.toDouble()); // <- НЕЛЬЗЯ для любого T

    // Более общий подход - через генерацию double и преобразование в T
    // Это работает, если T можно создать из double (например, через конструктор или оператор присваивания)
    // Для float это работает напрямую.
    // Для Fraction - нужно, чтобы был конструктор из double или int.
    // Мы модифицируем Fraction, чтобы он мог принимать double и создавать дробь (например, 3.14 -> 314/100)

    // uniform_real_distribution<double> dis(min_val, max_val); // <- НЕЛЬЗЯ, если min_val/max_val не float/double

    // Попробуем общий подход: используем min_val и max_val как T, но генерируем double между ними
    // Это требует, чтобы T поддерживал арифметику и преобразования.

    // Более надёжный способ - специализировать функцию для конкретных типов.
    // Но можно сделать общий случай, если T поддерживает арифметику и преобразования.

    // Мы реализуем общий случай, предполагая, что T поддерживает:
    // - operator- (для разности)
    // - operator+ (для сложения)
    // - умножение на double
    // - static_cast<T> от double

    // Для float/double это работает.
    // Для Fraction нужно будет добавить конструктор из double или operator= из double
    // или реализовать специализацию.

    // Пока реализуем общий случай, который будет работать для float/double и Fraction (с доработкой)
    // Используем генерацию int или double, если T поддерживает это.

    // Упрощённый общий случай: генерируем double, преобразуем в T.
    // Это не идеально, но работает для базовых типов и может работать для Fraction с правильным конструктором.
    // В Fraction добавим конструктор из double.

    static uniform_real_distribution<double> dis(0.0, 1.0); // Генерим от 0 до 1

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double t = dis(gen); // t от 0 до 1
            // Интерполируем между min_val и max_val
            // val = min_val + t * (max_val - min_val)
            T range = max_val - min_val;
            T val = min_val + static_cast<T>(t) * range; // static_cast<T> от double
            data[i][j] = val;
        }
    }
}
// --- КОНЕЦ НОВОЙ РЕАЛИЗАЦИИ ---

template<typename T>
void TMatrix<T>::modifyElement(int row, int col) {
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        cout << "Enter new value for [" << row << "][" << col << "]: ";
        cin >> data[row][col];
    } else {
        cout << "Error: Index out of bounds!" << endl;
    }
}

// --- НОВАЯ РЕАЛИЗАЦИЯ SetRandomValue ---
template<typename T>
void TMatrix<T>::SetRandomValue(int row, int col, T min_val, T max_val) {
    if (row < 0 || row >= rows || col < 0 || col >= cols) {
        cout << "Error: Index out of bounds!" << endl;
        return;
    }

    // Используем тот же подход, что и в GenerateRandomValues
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<double> dis(0.0, 1.0);

    double t = dis(gen);
    T range = max_val - min_val;
    T val = min_val + static_cast<T>(t) * range;
    data[row][col] = val;
}
// --- КОНЕЦ НОВОЙ РЕАЛИЗАЦИИ ---

template<typename T>
void TMatrix<T>::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }

    file >> rows >> cols;
    data.resize(rows, vector<T>(cols));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            file >> data[i][j];
        }
    }
    file.close();
    cout << "Data loaded from " << filename << endl;
}

template<typename T>
void TMatrix<T>::saveToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }

    file << rows << " " << cols << endl;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            file << data[i][j] << " ";
        }
        file << endl;
    }
    file.close();
    cout << "Data saved to " << filename << endl;
}

#endif