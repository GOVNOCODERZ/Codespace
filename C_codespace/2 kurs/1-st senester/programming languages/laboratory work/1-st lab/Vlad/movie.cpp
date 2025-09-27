#include "movie.h"
#include <iostream>
#include <vector> // для массивов
using namespace std;

Movie::Movie(): 
    name(""),
    director(""),
    year(0),
    genre(""),
    rating(0),
    length(0){}

Movie::Movie(
    string myName, 
    string myDirector, 
    int myYear, 
    string myGenre, 
    float myRating, 
    int myLength
): 
    name(myName),
    director(myDirector),
    year(myYear),
    genre(myGenre),
    rating(myRating),
    length(myLength){}

Movie::Movie(const Movie& other):
    name(other.name),
    director(other.director),
    year(other.year),
    genre(other.genre),
    rating(other.rating),
    length(other.length){}

Movie::~Movie() {}

/// @brief Ввод полей объекта ч/з поток
/// @return Поля объекта
istream& operator>>(
    istream& is, 
    Movie& mov
) {
    cout << "Enter name: ";
    is >> mov.name;
    cout << "Enter director: ";
    is >> mov.director;
    cout << "Enter year: ";
    is >> mov.year;
    cout << "Enter genre: ";
    is >> mov.genre;
    cout << "Enter rating: ";
    is >> mov.rating;
    cout << "Enter length: ";
    is >> mov.length;
    return is;
}

/// @brief Вывод полей объекта ч/з поток
/// @return Поля объекта
ostream& operator<<(
    ostream& os,
    const Movie& mov
) {
    os << "Name: \"" << mov.name << "\"" << endl
       << "Director: \"" << mov.director << "\"" << endl
       << "Year: \"" << mov.year << "\"" << endl
       << "Genre: \"" << mov.genre << "\"" << endl
       << "Rating: \"" << mov.rating << "\"" << endl
       << "Length: \"" << mov.length << "\"" << endl;
    return os;
}

/// @brief Выборка a) По фильмам режиссёра
/// @param movies массив объектов
/// @param director имя режиссёра
/// @return Отфильтрованный массив
vector<Movie> getMoviesByDirector(
    const vector<Movie>& movies, 
    const string& director
) {
    vector<Movie> result;
    for (const Movie& mov : movies) {
        if (mov.getDirector() == director) {
            result.push_back(mov);
        }
    }
    return result;
}

/// @brief Выборка б) По жанру + рейтинг больше искомого
/// @param movies массив объектов
/// @param genre жанр
/// @param minRating минимальный допустимый рейтинг
/// @return Отфильтрованный массив
vector<Movie> getMoviesByGenreAndRating(
    const vector<Movie>& movies, 
    const string& genre, 
    float minRating
) {
    vector<Movie> result;
    for (const Movie& mov : movies) {
        if (mov.getGenre() == genre && mov.getRating() > minRating) {
            result.push_back(mov);
        }
    }
    return result;
}

/// @brief Выборка в) год выхода в опр-м промежутке (включительно)
/// @param movies массив объектов
/// @param startYear ранний год
/// @param endYear поздний год
/// @return Отфильтрованный массив
vector<Movie> getMoviesBetweenYears(
    const vector<Movie>& movies, 
    int startYear, 
    int endYear
) {
    vector<Movie> result;
    for (const Movie& mov : movies) {
        if (mov.getYear() >= startYear && mov.getYear() <= endYear) {
            result.push_back(mov);
        }
    }
    return result;
}

/// @brief Компаратор для функции qsort
/// @param a Поинтер на первый объект
/// @param b Поинтер на второй объект
/// @return Целое число как результат сравнения
int compareMoviesByRating(
    const void* a, 
    const void* b
) {
    const Movie* movieA = (const Movie*)a;
    const Movie* movieB = (const Movie*)b;

    if (movieA->getRating() < movieB->getRating()) return -1;
    else if (movieA->getRating() > movieB->getRating()) return 1;
    else return 0;
}

/// @brief Чтение инфы из файла 
/// @param is Поток ввода
/// @param count Счётчик кол-ва объектов
/// @return Список прочитанных объектов 
Movie* Movie::readFromFile(
    istream& is, 
    int& count
) {
    is >> count;  // считываем количество фильмов

    if (count <= 0) {
        return nullptr;
    }

    Movie* movies = new Movie[count];
    for (int i = 0; i < count; ++i) {
        is >> movies[i];
    }

    return movies;
}

/// @brief Запись инфы в файл
/// @param os Поток вывода 
/// @param movies Список объектов для записи
/// @param count Счётчик кол-ва объектов
void Movie::writeToFile(
    ostream& os, 
    const Movie* movies, 
    int count
) {
    for (int i = 0; i < count; ++i) {
        os << movies[i] << endl;
    }
}