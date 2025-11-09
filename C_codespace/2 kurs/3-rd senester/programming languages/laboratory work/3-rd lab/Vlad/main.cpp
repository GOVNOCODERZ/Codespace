#include "TMatrix.h"
#include "Fraction.h"
#include <iostream>
using namespace std;

int main() {
    // Объекты для работы с разными типами
    TMatrix<float> float_matrix;
    TMatrix<Fraction> fraction_matrix;

    // Указатели на базовый тип, чтобы можно было переключаться между ними
    TMatrix<float>* current_float_matrix = nullptr;
    TMatrix<Fraction>* current_fraction_matrix = nullptr;

    // Переменная для выбора типа матрицы
    int matrix_type = 0; // 0 - нет, 1 - float, 2 - fraction

    bool running = true;

    while (running) {
        cout << "\n======================================" << endl;
        cout << "           TMatrix MENU" << endl;
        cout << "======================================" << endl;
        cout << "MATRIX TYPE: ";
        if (matrix_type == 1) cout << "CURRENTLY WORKING WITH FLOAT MATRIX" << endl;
        else if (matrix_type == 2) cout << "CURRENTLY WORKING WITH FRACTION MATRIX" << endl;
        else cout << "NO MATRIX SELECTED" << endl;

        cout << "\n--- MATRIX TYPE SELECTION ---" << endl;
        cout << "11. Switch to Float Matrix" << endl;
        cout << "12. Switch to Fraction Matrix" << endl;

        if (matrix_type != 0) {
            cout << "\n--- OPERATIONS ON CURRENT MATRIX ---" << endl;
            cout << "1. Create new matrix from input" << endl;
            cout << "2. Display current matrix" << endl;
            cout << "3. Count positive elements below row mean" << endl;
            cout << "4. Sum of negative elements" << endl;
            cout << "5. Load matrix from file" << endl;
            cout << "6. Save current matrix to file" << endl;
            cout << "7. Modify an element (console input)" << endl;
            cout << "8. Set random value for an element" << endl;
            cout << "9. Generate random values for the whole matrix" << endl;
            cout << "10. Demonstration of copy constructor" << endl;
        }

        cout << "0. Exit" << endl;
        cout << "Choose an action: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 11: { // Переключение на матрицу вещественных чисел
                matrix_type = 1;
                current_float_matrix = &float_matrix;
                current_fraction_matrix = nullptr; // Сбрасываем указатель на Fraction
                cout << "Switched to Float Matrix." << endl;
                break;
            }
            case 12: { // Переключение на матрицу дробей
                matrix_type = 2;
                current_fraction_matrix = &fraction_matrix;
                current_float_matrix = nullptr; // Сбрасываем указатель на Float
                cout << "Switched to Fraction Matrix." << endl;
                break;
            }


            case 1: { // Ввод матрицы
                if (matrix_type == 1) {
                    current_float_matrix->input();
                    cout << "Float matrix created." << endl;
                } else if (matrix_type == 2) {
                    current_fraction_matrix->input();
                    cout << "Fraction matrix created." << endl;
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 2: { // Вывод матрицы
                if (matrix_type == 1) {
                    cout << "\n--- Current Float Matrix ---" << endl;
                    cout << *current_float_matrix << endl;
                } else if (matrix_type == 2) {
                    cout << "\n--- Current Fraction Matrix ---" << endl;
                    cout << *current_fraction_matrix << endl;
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 3: { // Счёт положительных элементов > среднего значения
                if (matrix_type == 1) {
                    cout << "\nCount of positive elements below row mean: " << current_float_matrix->countPositiveBelowRowMean() << endl;
                } else if (matrix_type == 2) {
                    cout << "\nCount of positive elements below row mean: " << current_fraction_matrix->countPositiveBelowRowMean() << endl;
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 4: { // Сумма отрицательных элементов
                if (matrix_type == 1) {
                    auto sum = current_float_matrix->sumOfNegativeElements();
                    cout << "\nSum of negative elements: " << sum << endl;
                } else if (matrix_type == 2) {
                    auto sum = current_fraction_matrix->sumOfNegativeElements();
                    cout << "\nSum of negative elements: " << sum << endl;
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 5: { // Загрузка матрицы из файла
                string filename;
                cout << "Enter filename to load from: ";
                cin >> filename;
                if (matrix_type == 1) {
                    current_float_matrix->loadFromFile(filename);
                } else if (matrix_type == 2) {
                    current_fraction_matrix->loadFromFile(filename);
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 6: { // Выгрузка матрицы в файл
                string filename;
                cout << "Enter filename to save to: ";
                cin >> filename;
                if (matrix_type == 1) {
                    current_float_matrix->saveToFile(filename);
                } else if (matrix_type == 2) {
                    current_fraction_matrix->saveToFile(filename);
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 7: { // Модификация элемента
                int r, c;
                cout << "Enter row index: ";
                cin >> r;
                cout << "Enter column index: ";
                cin >> c;
                if (matrix_type == 1) {
                    current_float_matrix->modifyElement(r, c);
                } else if (matrix_type == 2) {
                    current_fraction_matrix->modifyElement(r, c);
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 8: { // Задать случайное значение элементу
                int r, c;
                cout << "Enter row index: ";
                cin >> r;
                cout << "Enter column index: ";
                cin >> c;
                if (matrix_type == 1) {
                    float min_val, max_val;
                    cout << "Enter min value for random: ";
                    cin >> min_val;
                    cout << "Enter max value for random: ";
                    cin >> max_val;
                    current_float_matrix->SetRandomValue(r, c, min_val, max_val);
                } else if (matrix_type == 2) {
                    Fraction min_val, max_val;
                    cout << "Enter min value for random: ";
                    cin >> min_val;
                    cout << "Enter max value for random: ";
                    cin >> max_val;
                    current_fraction_matrix->SetRandomValue(r, c, min_val, max_val);
                } else {
                    cout << "Please select a matrix type first." << endl;
                }
                break;
            }
            case 9: { // Сгенерировать случайные значения всем элементам матрицы
                switch (matrix_type){
                case 1:
                {
                    float min_val, max_val;
                    cout << "Enter min value for random: ";
                    cin >> min_val;
                    cout << "Enter max value for random: ";
                    cin >> max_val;
                    current_float_matrix->GenerateRandomValues(min_val, max_val);
                    cout << "Random values generated for Float Matrix." << endl;
                    break;
                }
                case 2:
                {
                    Fraction min_val, max_val;
                    cout << "Enter min value for random (as a/b): ";
                    cin >> min_val;
                    cout << "Enter max value for random (as a/b): ";
                    cin >> max_val;
                    current_fraction_matrix->GenerateRandomValues(min_val, max_val);
                    cout << "Random values generated for Fraction Matrix." << endl;
                    break;                    
                }
                default:
                    cout << "Please select a matrix type first." << endl;
                    break;
                }
                break;
            }
            case 10: { // Демонстрация конструктора копирования
                cout << "--- Demonstration of copy constructor ---" << endl;
                switch (matrix_type){
                case 1:
                {
                    cout << "Current Float Matrix" << endl;
                    cout << *current_float_matrix << endl;
                    TMatrix<float> temp_copy = *current_float_matrix;
                    cout << "Copy Of Current Float Matrix" << endl;
                    cout << temp_copy << endl;
                    break;
                }
                case 2:
                {
                    cout << "Current Fracture Matrix" << endl;
                    cout << *current_fraction_matrix << endl;
                    TMatrix<Fraction> temp_copy = *current_fraction_matrix;
                    cout << "Copy Of Current Fraction Matrix" << endl;
                    cout << temp_copy << endl;
                    break;
                }
                default:
                    cout << "Please select a matrix type first." << endl;
                    break;
                }
                break;
            }
            case 0: { // Выход из программы
                running = false;
                cout << "Exiting..." << endl;
                break;
            }
            default: { // Неверный ввод
                cout << "Invalid choice. Please try again." << endl;
            }
        }
    }

    return 0;
}