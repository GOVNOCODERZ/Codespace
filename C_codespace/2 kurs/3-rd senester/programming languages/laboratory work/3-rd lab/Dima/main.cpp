#include <iostream>
#include <fstream>
#include <ctime>
#include "Figures.h"
#include "TFigureArray.h"

using namespace std;

template <typename T>
void runMenu(TFigureArray<T>& array) {
    int choice;
    do {
        try {
            cout << "\n1. Показать | 2. Random | 3. Сортировка Шелла | 4. Выше среднего\n"
                 << "5. Изменить элемент | 6. Сохранить | 7. Загрузить | 0. Выход\nВыбор: ";
            if (!(cin >> choice)) throw FigureException("Введён неправильный параметр");

            switch (choice) {
                case 1: cout << array << endl; break;
                case 2: array.GenerateRandomValues(); break;
                case 3: array.shellSort(); break;
                case 4: array.findAboveAverage(); break;
                case 5: {
                    int idx; cout << "Индекс: "; cin >> idx;
                    cout << "Введите новые данные: "; cin >> array[idx]; 
                    break;
                }
                case 6: {
                    ofstream fout("output.txt");
                    if (!fout) throw FigureException("Ошибка записи в файл");
                    fout << array; break;
                }
                case 7: {
                    ifstream fin("input.txt");
                    if (!fin) throw FigureException("Файл не найден");
                    fin >> array; break;
                }
            }
        } 
        catch (const std::out_of_range& e) { cerr << "Ошибка границ: " << e.what() << endl; }
        catch (const ValidationError& e) { cerr << "Ошибка данных: " << e.what() << endl; }
        catch (const FigureException& e) { cerr << "Проблема: " << e.what() << endl; }
        catch (const std::exception& e) { cerr << "Системная ошибка: " << e.what() << endl; }
        
    } while (choice != 0);
}

int main() {
    srand(time(0));
    try {
        int type, n;
        cout << "Тип: 1-Квадраты, 2-Круги: "; cin >> type;
        cout << "Размер массива: "; cin >> n;

        if (type == 1) {
            TFigureArray<Square> sq(n);
            runMenu(sq);
        } else if (type == 2) {
            TFigureArray<Circle> circ(n);
            runMenu(circ);
        } else throw std::invalid_argument("Неверный тип фигур");
    }
    catch (const exception& e) {
        cerr << "Критическая ошибка: " << e.what() << endl;
    }
    return 0;
}