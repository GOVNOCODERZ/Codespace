#include "movie.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// File names for writing to/reading from
const string input_file_name = "input.txt";
const string output_file_name = "output.txt";

int main() {

    /// Массив фильмов
    vector<Movie> movies;
    /// Переменная для цикла while
    bool running = true;

    while (running) {
        cout << "\n======================================" << endl;
        cout << "               MAIN MENU" << endl;
        cout << "=======================================" << endl;
        cout << "1. Load data from file" << endl;
        cout << "2. Display data on screen" << endl;
        cout << "3. Save data to file" << endl;
        cout << "4. Select movies by director" << endl;
        cout << "5. Select movies by genre and rating" << endl;
        cout << "6. Select movies by years" << endl;
        cout << "7. Sort movies by rating (qsort)" << endl;
        cout << "8. Add new movie" << endl;
        cout << "9. Exit" << endl;
        cout << "\nChoose an action: ";

        int choice;
        cin >> choice;

        switch (choice) {
            /// Загрузка данных из файла
            case 1: {
                movies = Movie::read_from_file(input_file_name);
                break;
            }
            /// Отображение данных массива
            case 2: {
                if (movies.empty()) {
                    cout << ERROR_MESSAGE_EMPTY_LIST << endl;
                } else {
                    cout << "\n=== ALL MOVIES ===" << endl;
                    for (int i = 0; i < movies.size(); ++i) {
                        cout << movies[i] << endl;
                    }
                }
                break;
            }
            /// Запись данных в файл
            case 3: {
                Movie::write_to_file(output_file_name, movies);
                break;
            }
            /// Выборка а) по режиссёру
            case 4: {
                Movie::select_by_director(movies, movies.size());
                break;
            }
            /// Выборка б) по жанру и рейтингу
            case 5: {
                Movie::select_by_genre_and_rating(movies, movies.size());
                break;
            }
            /// Выборка в) по году выхода
            case 6: {
                Movie::select_by_years(movies, movies.size());
                break;
            }
            /// Сортировка по рейтингу (по убыванию)
            case 7: {
                if (!movies.empty())
                    Movie::sort_by_rating(movies);
                else 
                    cout << ERROR_MESSAGE_EMPTY_LIST << endl;
                break;
            }
            /// Добавление фильма в массив
            case 8: {
                Movie new_movie;
                cin >> new_movie;
                movies.push_back(new_movie);
                cout << "Movies added successfully." << endl;
                break;
            }
            /// Выход из программы
            case 9: {
                cout << "Exiting the program..." << endl;
                running = false;
                break;
            }
            /// Неверный выбор опции меню
            default:
                cout << ERROR_MESSAGE_INVALID_MENU_CHOICE << endl;
        }
    }

    return 0;
}