#include "movie.h"
#include <iostream>
#include <fstream>
using namespace std;

Movie::Movie(): 
    name(""),
    director(""),
    year(0),
    genre(""),
    rating(0),
    length(0) {}

Movie::Movie(string myName, string myDirector, int myYear, string myGenre, float myRating, int myLength): 
    name(myName),
    director(myDirector),
    year(myYear),
    genre(myGenre),
    rating(myRating),
    length(myLength) {}

Movie::Movie(const Movie& other):
    name(other.name),
    director(other.director),
    year(other.year),
    genre(other.genre),
    rating(other.rating),
    length(other.length) {}

Movie::~Movie() {}

/// @brief Ввод полей объекта ч/з поток. Проверки на соответствие потока нужны для правильной работы read_from_file
/// @param is Входящий поток
/// @param mov Поинтер на объект
/// @return Поля объекта в виде потока
istream& operator>>(istream& is, Movie& mov) {
    if (&is == &cin) {
        cout << "Enter name: ";
    }
    is >> mov.name;

    if (&is == &cin) {
        cout << "Enter director: ";
    }
    is >> mov.director;

    if (&is == &cin) {
        cout << "Enter year: ";
    }
    is >> mov.year;

    if (&is == &cin) {
        cout << "Enter genre: ";
    }
    is >> mov.genre;

    if (&is == &cin) {
        cout << "Enter rating: ";
    }
    is >> mov.rating;

    if (&is == &cin) {
        cout << "Enter length: ";
    }
    is >> mov.length;

    return is;
}


/// @brief Вывод полей объекта ч/з поток
/// @param os Выходящий поток
/// @param mov Поинтер на объект
/// @return Поля оьъекта в виде потока
ostream& operator<<(ostream& os, const Movie& mov) {
    os << "Name: \"" << mov.name << "\"" << endl
       << "Director: \"" << mov.director << "\"" << endl
       << "Year: \"" << mov.year << "\"" << endl
       << "Genre: \"" << mov.genre << "\"" << endl
       << "Rating: \"" << mov.rating << "\"" << endl
       << "Length: \"" << mov.length << "\"" << endl;
    return os;
}

/// @brief Чтение инфы из файла 
/// @param is Название файла для открытия
/// @param count Счётчик кол-ва объектов
/// @return Список прочитанных объектов 
Movie* Movie::read_from_file(const string& filename, int& count) {
    ifstream file(filename);
    file >> count;
    if (file.is_open()) {
        if (count <= 0) {
            return nullptr;
        }

        Movie* movies = new Movie[count];

        for (int i = 0; i < count; ++i) {
            file >> movies[i];
        }
        file.close();
        if (movies != nullptr) {
            cout << "Data loaded successfully. Number of movies: " << count << endl;
            return movies;
        }
    }
    else {
        cout << "Error loading data." << endl;
    }
    return nullptr;
}


/// @brief Запись инфы в файл
/// @param os Название файла для открытия
/// @param movies Список объектов для записи
/// @param count Счётчик кол-ва объектов
void Movie::write_to_file(const string& filename, const Movie* movies, int count) {
    ofstream file(filename);

    if (file.is_open()) {
        for (int i = 0; i < count; ++i) {
            file << movies[i] << endl;
        }
        file.close();
        cout << "Data saved to output.txt" << endl;
    }
    else {
        cout << "Could not open file for writing." << endl;
    }
}

/// @brief Компаратор для qsort
/// @param a Первый фильм для сравнения
/// @param b Второй фильм для сравнения
/// @return Результат сравнения как int
int Movie::compare_movies_by_rating(const void* a, const void* b) {
    const Movie* movieA = (const Movie*)a;
    const Movie* movieB = (const Movie*)b;

    if (movieA->getRating() > movieB->getRating()) return -1;  // в порядке убывания
    else if (movieA->getRating() < movieB->getRating()) return 1;
    else return 0;
}

/// @brief Выборка а) по режиссёру
/// @param movies Поинтер на массив
/// @param count Количество фильмов в массиве
void Movie::selectByDirector(Movie* movies, int count) {
    if (movies == nullptr || count == 0) {
        cout << "No data for selection." << endl;
        return;
    }

    string director;
    cout << "Enter director name: ";
    cin >> director;

    vector<Movie> selected;
    for (int i = 0; i < count; ++i) {
        if (movies[i].getDirector() == director) {
            selected.push_back(movies[i]);
        }
    }

    if (selected.empty()) {
        cout << "No movies found by this director." << endl;
    } else {
        cout << "\n=== MOVIES BY DIRECTOR \"" << director << "\" ===" << endl;
        for (const Movie& m : selected) {
            cout << m << endl;
        }
    }
}

/// @brief Выборка б) по жанру + рейтинг выше указанного
/// @param movies Поинтер на массив
/// @param count Количество фильмов в массиве
void Movie::selectByGenreAndRating(Movie* movies, int count) {
    if (movies == nullptr || count == 0) {
        cout << "No data for selection." << endl;
        return;
    }

    string genre;
    float minRating;
    cout << "Enter genre: ";
    cin >> genre;
    cout << "Enter minimum rating: ";
    cin >> minRating;

    vector<Movie> selected;
    for (int i = 0; i < count; ++i) {
        if (movies[i].getGenre() == genre && movies[i].getRating() > minRating) {
            selected.push_back(movies[i]);
        }
    }

    if (selected.empty()) {
        cout << "No movies found with this genre and rating." << endl;
    } else {
        cout << "\n=== MOVIES OF GENRE \"" << genre << "\" WITH RATING > " << minRating << " ===" << endl;
        for (const Movie& m : selected) {
            cout << m << endl;
        }
    }
}

/// @brief Выборка в) год выхода в указанном промежутке
/// @param movies Поинтер на массив
/// @param count Количество фильмов в массиве
void Movie::selectByYears(Movie* movies, int count) {
    if (movies == nullptr || count == 0) {
        cout << "No data for selection." << endl;
        return;
    }

    int startYear, endYear;
    cout << "Enter start year: ";
    cin >> startYear;
    cout << "Enter end year: ";
    cin >> endYear;

    vector<Movie> selected;
    for (int i = 0; i < count; ++i) {
        if (movies[i].getYear() >= startYear && movies[i].getYear() <= endYear) {
            selected.push_back(movies[i]);
        }
    }

    if (selected.empty()) {
        cout << "No movies found in the specified year range." << endl;
    } else {
        cout << "\n=== MOVIES BETWEEN " << startYear << " AND " << endYear << " ===" << endl;
        for (const Movie& m : selected) {
            cout << m << endl;
        }
    }
}