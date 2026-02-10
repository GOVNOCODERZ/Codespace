#ifndef TMATRIX_H
#define TMATRIX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
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

    // Конструктор перемещения
    TMatrix(TMatrix&& other) noexcept;

    // Деструктор
    ~TMatrix();

    // Оператор присваивания
    TMatrix& operator=(const TMatrix& other);

    // Оператор перемещающего присваивания
    TMatrix& operator=(TMatrix&& other) noexcept;

    // Оператор сравнения
    bool operator==(const TMatrix& other) const;

    // Методы ввода/вывода данных матрицы
    void input(istream& is = cin);
    void output(ostream& os = cout) const;

    // Перегрузки операторов ввода/вывода (обёртка соответствующих методов)
    friend istream& operator>>(istream& is, TMatrix& m) {
        if (is.fail()) {
            throw runtime_error("TMatrix Output: Failed to write dimensions to stream.");
        }          
        m.input(is);
        return is;
    }

    friend ostream& operator<<(ostream& os, const TMatrix& m) {
        if (os.fail()) {
            throw runtime_error("TMatrix Output: Failed to write dimensions to stream.");
        }        
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
    void modifyElement(int row, int col, istream& is);

    // Модификация элемента матрицы (случайное значение)
    void SetRandomValue(int row, int col, T min_val, T max_val);

    // Загрузка из файла
    void loadFromFile(const string& filename);

    // Сохранение в файл
    void saveToFile(const string& filename) const;

    // Проверка границы матрицы
    const T& at(int row, int col) const;
};

// --- РЕАЛИЗАЦИЯ МЕТОДОВ ШАБЛОНА ---

template<typename T>
TMatrix<T>::TMatrix() : rows(0), cols(0) {}

/**
 * @brief Конструктор с параметрами.
 * Создаёт матрицу размером `r x c`, заполняя элементы значением по умолчанию `T()`.
 * 
 * @param r Количество строк.
 * @param c Количество столбцов.
 * @throws `invalid_argument` если r < 0 или c < 0.
 * @throws `length_error` если `r * c` превышает максимальный размер `vector`.
 */
template<typename T>
TMatrix<T>::TMatrix(int r, int c) : rows(r), cols(c) {
    if (r < 0 || c < 0) {
        throw invalid_argument("TMatrix Constructor: Dimensions cannot be negative!");
    }
    // Проверка на переполнение при умножении (опционально, но безопасно)
    if (r > 0 && c > numeric_limits<int>::max() / r) {
         throw length_error("TMatrix Constructor: Dimensions are too large, would cause overflow!");
    }
    try {
        data.resize(rows, vector<T>(cols));
    }
    catch (const bad_alloc& e) { // Обработка исключения при выделении памяти
        // Возвращаем объект в корректное состояние (пустая матрица)
        rows = 0;
        cols = 0;
        throw;
    }
}

/**
 * @brief Конструктор копирования.
 * Создаёт новую матрицу, являющуюся копией переданной.
 * 
 * @param other Матрица, из которой производится копирование.
 * @throws `bad_alloc` при нехватке памяти для копирования.
 */
template<typename T>
TMatrix<T>::TMatrix(const TMatrix& other) : rows(other.rows), cols(other.cols) {
    try {
        data = other.data;
    }
    catch (const bad_alloc& e) {
        // Возвращаем объект в корректное состояние (пустая матрица)
        rows = 0;
        cols = 0;
        data.clear();
        throw;
    }
}

/**
 * @brief Конструктор перемещающего копирования.
 * Создаёт новую матрицу, являющуюся копией переданной.
 * 
 * @param other Матрица, из которой производится копирование.
 * @throws `bad_alloc` при нехватке памяти для копирования.
 */
template<typename T>
TMatrix<T>::TMatrix(TMatrix&& other) noexcept : data(move(other.data)), rows(other.rows), cols(other.cols) {
    other.rows = 0;
    other.cols = 0;
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
 * @brief Оператор перемещающего присваивания.
 * Присваивает значение полей одной матрицы другой.
 * При этом первая матрица очищается.
 * 
 * @param other Правый операнд присваивания.
 * @return Ссылку на текущий объект (*this).
 */
template<typename T>
TMatrix<T>& TMatrix<T>::operator=(TMatrix&& other) noexcept {
    if (this != &other) {
        data = move(other.data);
        rows = other.rows;
        cols = other.cols;

        other.rows = 0;
        other.cols = 0;
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
 * @brief Метод ввода матрицы из потока.
 * Сначала запрашивает количество строк и столбцов из потока.
 * Затем поэлементно запрашивает значения из потока.
 * @param is Входной поток.
 * @throws invalid_argument при вводе отрицательных размеров.
 * @throws runtime_error при ошибках потока.
 */
template<typename T>
void TMatrix<T>::input(istream& is) {
    cout << "Enter number of rows: ";
    is >> rows;
    if (is.fail()) {
        throw runtime_error("TMatrix Input: Failed to read number of rows from stream.");
    }
    if (rows < 0) {
        throw invalid_argument("TMatrix Input: Number of rows cannot be negative!");
    }
    cout << "Enter number of columns: ";
    is >> cols;
    if (is.fail()) {
        throw runtime_error("TMatrix Input: Failed to read number of columns from stream.");
    }
    if (cols < 0) {
        throw invalid_argument("TMatrix Input: Number of columns cannot be negative!");
    }

    try {
        data.resize(rows, vector<T>(cols));
    }
    catch (const bad_alloc& e) {
        throw;
    }

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << "Enter element [" << i << "][" << j << "]: ";
            is >> data[i][j];
            if (is.fail()) {
                throw runtime_error("TMatrix Input: Failed to read element [" + to_string(i) + "][" + to_string(j) + "] from stream.");
            }
        }
    }
}

/**
 * @brief Метод вывода матрицы в консоль.
 * Выводит элементы по строкам и столбцам в виде матрицы.
 * @param os Выходной поток.
 */
template<typename T>
void TMatrix<T>::output(ostream& os) const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            os << data[i][j] << " ";
            if (os.fail()) {
                throw runtime_error("TMatrix Output: Failed to write element [" + to_string(i) + "][" + to_string(j) + "] to stream.");
            }
        }
        os << endl;
        if (os.fail()) {
            throw runtime_error("TMatrix Output: Failed to write newline after row " + to_string(i) + " to stream.");
        }
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
 * 
 * @param min_val Минимальное значение для генерации.
 * @param max_val Максимальное значение для генерации.
 * @throws `invalid_argument` если `min_val > max_val`.
 */
template<typename T>
void TMatrix<T>::GenerateRandomValues(T min_val, T max_val) {
    if (min_val > max_val) {
        throw invalid_argument("TMatrix GenerateRandomValues: min_val cannot be greater than max_val!");
    }
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
 * @param is Входной поток.
 * @throws out_of_range если индексы выходят за границы.
 * @throws runtime_error при ошибках потока.
 */
template<typename T>
void TMatrix<T>::modifyElement(int row, int col, istream& is) {
    try { // Проверяем границы матрицы
        data.at(row).at(col);
        cout << "Enter new value for [" << row << "][" << col << "]: ";
        is >> data[row][col];
    }
    catch (const out_of_range&) {
        throw;
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
 * @throws out_of_range если индексы выходят за границы.
 * @throws invalid_argument если min_val > max_val.
 */
template<typename T>
void TMatrix<T>::SetRandomValue(int row, int col, T min_val, T max_val) {
    if (min_val > max_val) {
        throw invalid_argument("TMatrix SetRandomValue: min_val cannot be greater than max_val!");
    }
    try { // Проверяем границы матрицы
        data.at(row).at(col);
    }
    catch (const out_of_range&) {
        throw;
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
 * @throws runtime_error при ошибках файла.
  */
template<typename T>
void TMatrix<T>::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("TMatrix loadFromFile: Cannot open file " + filename);
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
 * @throws runtime_error при ошибках файла.
 */
template<typename T>
void TMatrix<T>::saveToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("TMatrix saveToFile: Cannot open file " + filename);
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

template<typename T>
const T& TMatrix<T>::at(int row, int col) const {
    return data.at(row).at(col);
}

#endif