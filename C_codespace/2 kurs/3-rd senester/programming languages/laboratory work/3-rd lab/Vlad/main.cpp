#include "TMatrix.h"
#include "Fraction.h"
#include "TMatrix.cpp"
#include <iostream>
using namespace std;

//TODO: СНАЧАЛА МЕНЮ ВЫБОРА ТИПА МАТРИЦЫ ДЛЯ ДЕМОНСТРАЦИИ, ЗАТЕМ ВЫЗЫВАЕТСЯ ФУНКЦИЮ С МЕНЮ НА КАЖДЫЙ Т.Д. (А ПОТОМ ЭТИ ВСЕ КОСТЫЛИ В НЕЙКРОНКУ ДЛЯ ФИКСА ЗАСУНУТЬ)

template <typename T>
void demonstrate(const string& type_name) {
    TMatrix<T> matrix;
    bool running = true;

    while (running) {
        cout << "\n--- Menu for \"" << type_name << "\" type matrix ---\n";
        cout << "1. Create new matrix from input" << endl;
        cout << "2. Display current matrix" << endl;
        cout << "3. Count positive elements below row mean" << endl;
        cout << "4. Sum of negative elements" << endl;
        cout << "5. Load matrix from file" << endl;
        cout << "6. Save current matrix to file" << endl;
        cout << "7. Modify an element (console input)" << endl;
        cout << "8. Set random value for an element" << endl;
        cout << "9. Generate random values for the whole matrix" << endl;
        cout << "0. Exit" << endl;
        cout << "Choose an action: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                cout << "\n--- Creating new matrix from input ---" << endl;
                cin >> matrix;
                break;
            }
            case 2: {
                cout << "\n--- Current Matrix ---" << endl;
                cout << matrix << endl;
                break;
            }
            case 3: {
                auto count = matrix.countPositiveBelowRowMean();
                cout << "\nCount of positive elements below row mean: " << count << endl;
                break;
            }
            case 4: {
                auto sum = matrix.sumOfNegativeElements();
                cout << "\nSum of negative elements: " << sum << endl;
                break;
            }
            case 5: {
                string filename;
                cout << "Enter filename to load from: ";
                cin >> filename;
                matrix.loadFromFile(filename);
                break;
            }
            case 6: {
                string filename;
                cout << "Enter filename to save to: ";
                cin >> filename;
                matrix.saveToFile(filename);
                break;
            }
            case 7: {
                int r, c;
                cout << "Enter row index: ";
                cin >> r;
                cout << "Enter column index: ";
                cin >> c;
                matrix.modifyElement(r, c);
                break;
            }
            case 8: {
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
            case 9: {
                T min_val, max_val;
                cout << "Enter min value for random: ";
                cin >> min_val;
                cout << "Enter max value for random: ";
                cin >> max_val;
                matrix.GenerateRandomValues(min_val, max_val);
                cout << "Random values generated." << endl;
                break;
            }
            case 0: {
                running = false;
                cout << "Exiting..." << endl;
                break;
            }
            default: {
                cout << "Invalid choice. Please try again." << endl;
            }
        }
    }
}

int main() {
    while (true) {
        cout << "\n======================================" << endl;
        cout << "           MAIN MENU" << endl;
        cout << "========================================" << endl;
        cout << "Choose data type to work with:\n";
        cout << "1. Float\n";
        cout << "2. Fraction\n";
        cout << "0. Exit\n";
        cout << "Choose an action: ";
        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                demonstrate<float>("Float");
                break;
            case 2:
                demonstrate<Fraction>("Fraction");
                break;
            case 0:
                cout << "Exiting...\n";
                return 0;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
    return 0;
}