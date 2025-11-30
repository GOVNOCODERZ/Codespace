#include "TMatrix.h"
#include "Fraction.h"
#include <iostream>
#include <fstream>
using namespace std;

/// @brief Шаблонная функция, реализующая меню для конкретного типа данных
/// @tparam T тип данных элементов матрицы
/// @param datatype_name название типа данных
template<typename T>
void runMatrixMenu(string datatype_name) {
    TMatrix<T> matrix;
    bool running = true;

    while (running) {
        cout << "\n======================================" << endl;
        cout << "     TMatrix MENU (Type: " << datatype_name << ")" << endl;
        cout << "======================================" << endl;
        cout << "\n--- OPERATIONS ON MATRIX ---" << endl;
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
        cout << "11. Demonstration of move constructor" << endl;
        cout << "0. Go back to type selection" << endl;
        cout << "Choose an action: ";

        int choice;
        cin >> choice;

        try {
            switch (choice) {
                case 1: { // Ввод матрицы
                    matrix.input();
                    cout << "Matrix created." << endl;
                    break;
                }
                case 2: { // Вывод матрицы
                    cout << "\n--- Current Matrix ---" << endl;
                    cout << matrix << endl;
                    break;
                }
                case 3: { // Счёт положительных элементов > среднего значения
                    cout << "\nCount of positive elements below row mean: " << matrix.countPositiveBelowRowMean() << endl;
                    break;
                }
                case 4: { // Сумма отрицательных элементов
                    auto sum = matrix.sumOfNegativeElements();
                    cout << "\nSum of negative elements: " << sum << endl;
                    break;
                }
                case 5: { // Загрузка матрицы из файла
                    string filename;
                    cout << "Enter filename to load from: ";
                    cin >> filename;
                    matrix.loadFromFile(filename);
                    break;
                }
                case 6: { // Выгрузка матрицы в файл
                    string filename;
                    cout << "Enter filename to save to: ";
                    cin >> filename;
                    matrix.saveToFile(filename);
                    break;
                }
                case 7: { // Модификация элемента
                    int r, c;
                    cout << "Enter row index: ";
                    cin >> r;
                    cout << "Enter column index: ";
                    cin >> c;
                    matrix.modifyElement(r, c, cin);
                    break;
                }
                case 8: { // Задать случайное значение элементу
                    int r, c;
                    cout << "Enter row index: ";
                    cin >> r;
                    cout << "Enter column index: ";
                    cin >> c;
                    T min_val, max_val;
                    cout << "Enter min value for random: ";
                    cin >> min_val;
                    cout << "Enter max value for random: ";
                    cin >> max_val;
                    matrix.SetRandomValue(r, c, min_val, max_val);
                    break;
                }
                case 9: { // Сгенерировать случайные значения всем элементам матрицы
                    T min_val, max_val;
                    cout << "Enter min value for random: ";
                    cin >> min_val;
                    cout << "Enter max value for random: ";
                    cin >> max_val;
                    matrix.GenerateRandomValues(min_val, max_val);
                    cout << "Random values generated." << endl;
                    break;
                }
                case 10: { // Демонстрация конструктора копирования
                    cout << "--- Demonstration of copy constructor ---" << endl;
                    cout << "Current Matrix" << endl;
                    cout << matrix << endl;
                    TMatrix<T> temp_copy = matrix;
                    cout << "Copy Of Current Matrix" << endl;
                    cout << temp_copy << endl;
                    break;
                }
                case 11: { // Демонстрация конструктора перемещения
                    cout << "--- Demonstration of move constructor ---" << endl;
                    cout << "Current Matrix" << endl;
                    cout << matrix << endl;
                    TMatrix<T> temp_moved = std::move(matrix);
                    cout << "Matrix after move (should be empty)" << endl;
                    cout << matrix << endl;
                    cout << "Moved Matrix" << endl;
                    cout << temp_moved << endl;
                    // Восстанавливаем оригинальную матрицу для дальнейшей работы
                    matrix = std::move(temp_moved);
                    cout << "Original matrix restored." << endl;
                    break;
                }
                case 0: { // Возврат к выбору типа
                    running = false;
                    cout << "Returning to type selection..." << endl;
                    break;
                }
                default: { // Неверный ввод
                    cout << "Invalid choice. Please try again." << endl;
                }
            }
        }
        catch (const FractionException& e) {
            cout << "Fraction Error: " << e.what() << endl;
        }
        catch (const std::invalid_argument& e) {
            cout << "Invalid Argument Error: " << e.what() << endl;
        }
        catch (const std::runtime_error& e) {
            cout << "Runtime Error: " << e.what() << endl;
        }
        catch (const std::out_of_range& e) {
            cout << "Out of Range Error: " << e.what() << endl;
        }
        catch (const std::length_error& e) {
            cout << "Length Error (overflow): " << e.what() << endl;
        }
        catch (const std::bad_alloc& e) {
            cout << "Memory Allocation Error: " << e.what() << endl;
        }
        catch (...) {
            cout << "An unknown error occurred." << endl;
        }
    }
}

int main() {
    try {
        bool running = true;

        while (running) {
            cout << "\n======================================" << endl;
            cout << "         MAIN MENU" << endl;
            cout << "======================================" << endl;
            cout << "\n--- SELECT MATRIX TYPE ---" << endl;
            cout << "1. Work with Float Matrix" << endl;
            cout << "2. Work with Fraction Matrix" << endl;
            cout << "0. Exit" << endl;
            cout << "Choose an action: ";

            int choice;
            cin >> choice;

            switch (choice) {
                case 1: { // Переключение на матрицу вещественных чисел
                    cout << "Switched to Float Matrix." << endl;
                    runMatrixMenu<float>("Float"); // Вызываем шаблонную функцию с типом float
                    break;
                }
                case 2: { // Переключение на матрицу дробей
                    cout << "Switched to Fraction Matrix." << endl;
                    runMatrixMenu<Fraction>("Fraction"); // Вызываем шаблонную функцию с типом Fraction
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
    }
    catch (const std::exception& e) {
        // Перехватываем любые стандартные исключения, вышедшие из основного цикла (т. е. критические)
        cout << "A critical error occurred in the main application: " << e.what() << endl;
        cout << "Application will be terminated." << endl;
        return 1; // Возвращаем код ошибки
    }
    catch (...) {
        // Перехватываем любые другие исключения (т. е. критические)
        cout << "An unknown critical error occurred in the main application." << endl;
        cout << "Application will be terminated." << endl;
        return 1; // Возвращаем код ошибки
    }

    return 0;
}