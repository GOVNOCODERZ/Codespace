#include "movie.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdlib>
using namespace std;

// File names for writing to/reading from
const string input_file_name = "input.txt";
const string output_file_name = "output.txt";

/// @brief Метод для добавления объекта в массив
/// @param movies Массив для добавления
/// @param count Длина массива
void addMovie(Movie*& movies, int& count) {
    cout << "\n=== ADDING A NEW MOVIE ===" << endl;
    Movie newMovie;
    cin >> newMovie;

    Movie* newMovies = new Movie[count + 1];
    for (int i = 0; i < count; ++i) {
        newMovies[i] = movies[i];
    }
    newMovies[count] = newMovie;

    if (movies != nullptr) {
        delete[] movies;
    }

    movies = newMovies;
    count++;
    cout << "Movie added successfully!" << endl;
}

int main() {
    /// Массив фильмов
    Movie* movies = nullptr;
    /// Счётчик фильмов в массиве
    int movieCount = 0;
    /// Переменная для цикла while
    bool running = true;

    while (running) {
        cout << "\n======================================" << endl;
        cout << "               MAIN MENU" << endl;
        cout << "======================================" << endl;
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
            case 1: {
                movies = Movie::read_from_file(input_file_name, movieCount);
                break;
            }

            case 2: {
                if (movies == nullptr || movieCount == 0) {
                    cout << "No data to display." << endl;
                } else {
                    cout << "\n=== ALL MOVIES ===" << endl;
                    for (int i = 0; i < movieCount; ++i) {
                        cout << movies[i] << endl;
                    }
                }
                break;
            }

            case 3: {
                if (movies == nullptr || movieCount == 0) {
                    cout << "No data to save." << endl;
                }
                else {
                    Movie::write_to_file(output_file_name, movies, movieCount);
                }
                break;
            }

            case 4:
                selectByDirector(movies, movieCount);
                break;

            case 5:
                selectByGenreAndRating(movies, movieCount);
                break;

            case 6:
                selectByYears(movies, movieCount);
                break;

            case 7: {
                if (movies == nullptr || movieCount == 0) {
                    cout << "No data to sort." << endl;
                } else {
                    cout << "Sorting by rating (descending) using qsort..." << endl;
                    qsort(movies, movieCount, sizeof(Movie), compare_movies_by_rating);
                    cout << "Sorting completed." << endl;
                }
                break;
            }

            case 8: {
                addMovie(movies, movieCount);
                break;
            }

            case 9: {
                cout << "Exiting the program..." << endl;
                running = false;
                break;
            }

            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }

    // Очищаем память перед окончанием программы
    if (movies != nullptr) {
        delete[] movies;
    }

    return 0;
}