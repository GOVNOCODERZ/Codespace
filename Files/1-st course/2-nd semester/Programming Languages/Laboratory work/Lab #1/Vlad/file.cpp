#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

using namespace std;

template <typename T>
void fillMatrix(T matrix[][100], int rows, int cols, int k) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<T>(rand() % (2 * abs(k) + 1) - abs(k));
        }
    }
}

template <typename T>
void printMatrix(const T matrix[][100], int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << setw(5) << matrix[i][j];
        }
        cout << endl;
    }
}

template <typename T>
int swapColumns(T matrix[][100], int rows, int cols, int M1, int M2) {
    if (M1 > 0 && M1 <= cols && M2 > 0 && M2 <= cols) {
        for (int i = 0; i < rows; ++i) {
            swap(matrix[i][M1-1], matrix[i][M2-1]);
        }
        return 1;
    }
    else {
        cout << "Ошибка: некорректные номера столбцов!" << endl;
    }
    return 0;
}

int main() {
    int rows, cols;
    double k;

    
    srand(static_cast<unsigned>(time(0)));

    cout << "Введите количество строк: ";
    cin >> rows;
    cout << "Введите количество столбцов: ";
    cin >> cols;
    cout << "Введите значение параметра k в диапазоне [-k, k]: ";
    cin >> k;

    double matrix[100][100];

    fillMatrix(matrix, rows, cols, k);

    cout << "Исходная матрица:" << endl;
    printMatrix(matrix, rows, cols);

    int M1, M2;
    cout << "Введите номер первого столбца M1 (от 1 до " << cols <<"): ";
    cin >> M1;
    cout << "Введите номер второго столбца M2 (от 1 до " << cols <<"): ";
    cin >> M2;

    swapColumns(matrix, rows, cols, M1, M2);

    cout << "Матрица после обмена столбцов: " << endl;
    printMatrix(matrix, rows, cols);

    return 0;
}