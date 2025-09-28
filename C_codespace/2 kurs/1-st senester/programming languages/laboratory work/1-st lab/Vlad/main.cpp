#include "movie.h"
#include <iostream>
#include <cstdlib> // для qsort из C
#include <vector> // для массивов
using namespace std;

/*
 * Сообщения при ошибке программы
 */

#define ERROR_MESSAGE_EMPTY_LIST "ERROR: The list is empty."
#define ERROR_MESSAGE_NOT_A_NUM "ERROR: Inserted text isn't a number."
#define ERROR_MESSAGE_INVALID_MENU_CHOICE "ERROR: Invalid menu option."

int main() {

/// Массив объектов
vector<Movie> movies;
/// Поток ввода
ifstream in_file("movies.txt");
/// Поток вывода
ofstream out_file("movies.txt");

/// Выбор юзера в меню
int choice;

do {
    cout << "\n========= MENU =========\n"
        << "1. Load from file" << endl
        << "2. Save to file" << endl
        << "3. Show all movies" << endl
        << "4. Add a new movie" << endl
        << "5. Sort by rating (descending)" << endl
        << "6. Filter by director's name" << endl
        << "7. Filter by genre + rating > some minimal rating" << endl
        << "8. Filter by a year in some borders" << endl
        << "0. Exit"  << endl
        << "========================" << endl
        << "Your choice: ";

    cin >> choice;
    if (cin.fail()) {
        cout << ERROR_MESSAGE_NOT_A_NUM << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        continue;
    }
    
    /// Обработка выбранных опций меню
    switch (choice) {
        /// Чтение из файла
        case 1: {
            movies = Movie::readFromFile(ifstream);
            break;
        }
        /// Запись в файл
        case 2: {
            Movie::writeToFile(ofstream, movies, );
            break;
        }
        /// Вывод массива фильмов
        case 3: {
            if (movies.empty()) {
                cout << ERROR_MESSAGE_EMPTY_LIST << endl;
            } else {
                for (const auto& mov : movies) {
                    cout << "--------------------\n";
                    mov.print(cout);
                }
            }
            break;
        }
        /// Добавление нового фильма в массив
        case 4: {
            Movie newMov;
            newMov.read(cin);
            movies.push_back(newMov);
            cout << "Movie added successfully.\n";
            break;
        }
        /// Сортировка фильмов в массиве по рейтингу (qsort'ом)
        case 5: {
            if (!movies.empty()) sortMovloyees(movies);
            else cout << ERROR_MESSAGE_EMPTY_LIST << endl;
            break;
        }
        /// Выборка a) По фильмам режиссёра
        case 6:
            if (!movies.empty()) findByDepartment(movies);
            else cout << ERROR_MESSAGE_EMPTY_LIST << endl;
            break;
        /// Выборка б) По жанру + рейтинг больше искомого
        case 7:
                if (!movies.empty()) findAboveAverageSalary(movies);
                else cout << ERROR_MESSAGE_EMPTY_LIST << endl;
            break;
        /// Выборка в) год выхода в опр-м промежутке (включительно)
        case 8:
                if (!movies.empty()) findWithExperience(movies);
                else cout << ERROR_MESSAGE_EMPTY_LIST << endl;
            break;
        /// Закрытие программы
        case 0:
            cout << "Exiting the program...\n";
            break;
        default:
            cout << ERROR_MESSAGE_INVALID_MENU_CHOICE << endl;
            break;
    }
} while (choice != 0);

return 0;
}

#undef ERROR_MESSAGE_EMPTY_LIST
#undef ERROR_MESSAGE_NOT_A_NUM
#undef ERROR_MESSAGE_INVALID_MENU_CHOICE