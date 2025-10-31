#include "TMatrix.h"
#include <limits>
#include <algorithm>

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
        T mean = sum / cols; // Деление на int автоматически преобразуется

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

template<typename T>
void TMatrix<T>::GenerateRandomValues(T min_val, T max_val) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dis(min_val.toDouble(), max_val.toDouble());

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double val = dis(gen);
            data[i][j] = T(static_cast<int>(val * 100), 100); // Пример для Fraction
        }
    }
}

template<typename T>
void TMatrix<T>::modifyElement(int row, int col) {
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        cout << "Enter new value for [" << row << "][" << col << "]: ";
        cin >> data[row][col];
    } else {
        cout << "Error: Index out of bounds!" << endl;
    }
}

template<typename T>
void TMatrix<T>::SetRandomValue(int row, int col, T min_val, T max_val) {
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> dis(min_val.toDouble(), max_val.toDouble());

        double val = dis(gen);
        data[row][col] = T(static_cast<int>(val * 100), 100); // Пример для Fraction
    } else {
        cout << "Error: Index out of bounds!" << endl;
    }
}

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