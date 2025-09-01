int mass[1] = {i+1; j}


// Вариант 17. Написать программу сравнения двух матриц одинакового размера на равенство. 
// Выделим функции: 
// 1.	Заполнение матрицы числами с клавиатуры.
// 2.	Вывод матрицы в матричном виде.
// 3.	Сравнение двух матриц на равенство. Результат – номер первой строки, где обнаружились не равные элементы
#include <iostream>
#include <vector>
 
template <typename Type>
void fillMatrix(std::vector<std::vector<Type>>& matrix, int rows, int cols) {
    std::cout << "Введите элементы матрицы (" << rows << "x" << cols << "):\n";
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cin >> matrix[i][j];
        }
    }
}
 
template <typename Type>
void printMatrix(const std::vector<std::vector<Type>>& matrix) {
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
}
 
template <typename Type>
int compareMatrices(const std::vector<std::vector<Type>>& matrix1, const std::vector<std::vector<Type>>& matrix2) {
    for (int i = 0; i < matrix1.size(); ++i) {
        for (int j = 0; j < matrix1[i].size(); ++j) {
            if (matrix1[i][j] != matrix2[i][j]) {
                return i + 1;
            }
        }
    }
    return -1; 
}
 
int main() {
    setlocale(LC_ALL, "ru");
    int rows, cols;
    std::cout << "Введите количество строк и столбцов: ";
    std::cin >> rows >> cols;
 
    std::vector<std::vector<double>> matrix1(rows, std::vector<double>(cols));
    std::vector<std::vector<double>> matrix2(rows, std::vector<double>(cols));
 
    std::cout << "Заполнение первой матрицы:\n";
    fillMatrix(matrix1, rows, cols);
 
    std::cout << "Заполнение второй матрицы:\n";
    fillMatrix(matrix2, rows, cols);
 
    std::cout << "Первая матрица:\n";
    printMatrix(matrix1);
 
    std::cout << "Вторая матрица:\n";
    printMatrix(matrix2);
 
    int result = compareMatrices(matrix1, matrix2);
    if (result == -1) {
        std::cout << "Матрицы равны.\n";
    }
    else {
        std::cout << "Матрицы отличаются. Первая несовпадающая строка: " << result << "\n";
    }
 
    return 0;
}