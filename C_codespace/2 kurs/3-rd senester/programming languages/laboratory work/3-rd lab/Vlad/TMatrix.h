#ifndef TMATRIX_H
#define TMATRIX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <limits>
#include <algorithm>
using namespace std;

/**
 * @brief Шаблонный класс, реализующий динамический двумерный массив
 * и предоставляющий методы для работы с ним.
 * 
 * @tparam T Тип элементов матрицы:
 * float (вещественное число)
 * Fraction (дробь вида a/b)
 */
template<typename T>
class TMatrix {
private:
    // Представление матрицы как вектора векторов (двумерный массив)
    vector<vector<T>> data;
    // Количество строк матрицы
    int rows;
    // Количество столбцов матрицы
    int cols;

public:
    // Конструктор по умолчанию (пустая матрица 0x0)
    TMatrix();

    // Конструктор с размерами (r x c)
    TMatrix(int r, int c);

    // Конструктор копирования
    TMatrix(const TMatrix& other);

    // Деструктор
    ~TMatrix();

    // Оператор присваивания
    TMatrix& operator=(const TMatrix& other);

    // Оператор сравнения
    bool operator==(const TMatrix& other) const;

    // Методы ввода/вывода данных матрицы
    void input();
    void output() const;

    // Перегрузки операторов ввода/вывода (обёртка соответствующих методов)
    friend istream& operator>>(istream& is, TMatrix& m) {
        m.input();
        return is;
    }

    friend ostream& operator<<(ostream& os, const TMatrix& m) {
        m.output();
        return os;
    }

    // Подсчёт элементов > 0 и < среднего по строке
    int countPositiveBelowRowMean() const;
    // Подсчёт суммы отрицательных элементов
    T sumOfNegativeElements() const;

    // Заполнение матрицы случайными значениями
    void GenerateRandomValues(T min_val, T max_val);

    // Модификация элемента матрицы (ввод с консоли)
    void modifyElement(int row, int col);

    // Модификация элемента матрицы (случайное значение)
    void SetRandomValue(int row, int col, T min_val, T max_val);

    // Загрузка из файла
    void loadFromFile(const string& filename);

    // Сохранение в файл
    void saveToFile(const string& filename) const;
};

// --- РЕАЛИЗАЦИЯ МЕТОДОВ ШАБЛОНА ---

template<typename T>
TMatrix<T>::TMatrix() : rows(0), cols(0) {}

/**
 * @brief Конструктор с параметрами.
 * Создаёт матрицу размером r x c, заполняя элементы значением по умолчанию T().
 * 
 * @param r Количество строк.
 * @param c Количество столбцов.
 */
template<typename T>
TMatrix<T>::TMatrix(int r, int c) : rows(r), cols(c) {
    data.resize(rows, vector<T>(cols));
}

/**
 * @brief Конструктор копирования.
 * Создаёт новую матрицу, являющуюся копией переданной.
 * 
 * @param other Матрица, из которой производится копирование.
 */
template<typename T>
TMatrix<T>::TMatrix(const TMatrix& other) : rows(other.rows), cols(other.cols) {
    data = other.data;
}

/**
 * @brief Деструктор.
 */
template<typename T>
TMatrix<T>::~TMatrix() {}

/**
 * @brief Оператор присваивания.
 * Присваивает значение полей одной матрицы другой.
 * 
 * @param other Правый операнд присваивания.
 * @return Ссылку на текущий объект (*this).
 */
template<typename T>
TMatrix<T>& TMatrix<T>::operator=(const TMatrix& other) {
    if (this != &other) {
        rows = other.rows;
        cols = other.cols;
        data = other.data;
    }
    return *this;
}

/**
 * @brief Оператор сравнения.
 * Проверяет, равны ли две матрицы (по размеру и содержимому).
 * 
 * @param other Матрица для сравнения.
 * @return true, если матрицы равны, иначе false.
 */
template<typename T>
bool TMatrix<T>::operator==(const TMatrix& other) const {
    return (rows == other.rows && cols == other.cols && data == other.data);
}

/**
 * @brief Метод ввода матрицы с консоли.
 * Сначала запрашивает количество строк и столбцов.
 * Затем поэлементно запрашивает значения.
 */
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

/**
 * @brief Метод вывода матрицы в консоль.
 * Выводит элементы по строкам и столбцам в виде матрицы.
 */
template<typename T>
void TMatrix<T>::output() const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << data[i][j] << " ";
        }
        cout << endl;
    }
}

/**
 * @brief Подсчёт элементов > 0 и < среднего по строке.
 * Для каждой строки вычисляется среднее арифметическое.
 * Затем подсчитывается количество элементов в строке, которые удовлетворяют условиям.
 * 
 * @return Общее количество таких элементов во всей матрице.
 */
template<typename T>
int TMatrix<T>::countPositiveBelowRowMean() const {
    int count = 0; // Количество нужных элементов
    for (int i = 0; i < rows; ++i) {
        T sum = T(0); // Сумма элементов строки
        for (int j = 0; j < cols; ++j) {
            sum = sum + data[i][j];
        }
        T mean = sum / static_cast<T>(cols); // Среднее арифметическое по строке

        for (int j = 0; j < cols; ++j) {
            if (data[i][j] > T(0) && data[i][j] < mean) {
                count++;
            }
        }
    }
    return count;
}

/**
 * @brief Расчёт суммы отрицательных элементов.
 * Перебирает все элементы матрицы и суммирует только те, что < T(0).
 * 
 * @return Сумму отрицательных элементов нужного типа данных.
 */
template<typename T>
T TMatrix<T>::sumOfNegativeElements() const {
    T sum = T(0); // Сумма подходящих элементов
    /// Проходимся по каждому элементу и прибавляем значение к сумме, если меньше нуля
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (data[i][j] < T(0)) {
                sum = sum + data[i][j];
            }
        }
    }
    return sum;
}

/**
 * @brief Метод генерации случайных значений.
 * Заполняет матрицу случайными значениями в диапазоне `[min_val, max_val]`.
 * Использует линейную интерполяцию: `val = min_val + t * (max_val - min_val)`,
 * где `t` — случайное число от 0 до 1.
 * 
 * @param min_val Минимальное значение для генерации.
 * @param max_val Максимальное значение для генерации.
 */
template<typename T>
void TMatrix<T>::GenerateRandomValues(T min_val, T max_val) {
    // Используем статическую переменную для инициализации генератора один раз
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<double> dis(0.0, 1.0); // Генерим от 0 до 1

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double t = dis(gen);
            T range = max_val - min_val;
            T val = min_val + static_cast<T>(t) * range;
            data[i][j] = val;
        }
    }
}

/**
 * @brief Модификация элемента матрицы вручную.
 * Запрашивает новое значение для элемента по указанным индексам.
 * 
 * @param row Индекс строки.
 * @param col Индекс столбца.
 */
template<typename T>
void TMatrix<T>::modifyElement(int row, int col) {
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        cout << "Enter new value for [" << row << "][" << col << "]: ";
        cin >> data[row][col];
    } else {
        cout << "ERROR: Index out of bounds!" << endl;
    }
}

/**
 * @brief Установка случайного значения для одного элемента.
 * Аналогично GenerateRandomValues, но только для одного элемента.
 * 
 * @param row Индекс строки.
 * @param col Индекс столбца.
 * @param min_val Минимальное значение.
 * @param max_val Максимальное значение.
 */
template<typename T>
void TMatrix<T>::SetRandomValue(int row, int col, T min_val, T max_val) {
    if (row < 0 || row >= rows || col < 0 || col >= cols) {
        cout << "ERROR: Index out of bounds!" << endl;
        return;
    }

    // Используем статическую переменную для инициализации генератора один раз
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<double> dis(0.0, 1.0); // Генерим от 0 до 1

    double t = dis(gen);
    T range = max_val - min_val;
    T val = min_val + static_cast<T>(t) * range;
    data[row][col] = val;
}

/**
 * @brief Загрузка матрицы из файла.
 * Формат файла: первые два числа — количество строк и столбцов.
 * Затем идут элементы по строкам.
 * 
 * @param filename Имя файла для загрузки.
 */
template<typename T>
void TMatrix<T>::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "ERROR: Cannot open file " << filename << endl;
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

/**
 * @brief Сохранение матрицы в файл.
 * Формат файла: первые два числа — количество строк и столбцов.
 * Затем идут элементы по строкам.
 * 
 * @param filename Имя файла для сохранения.
 */
template<typename T>
void TMatrix<T>::saveToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "ERROR: Cannot open file " << filename << endl;
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