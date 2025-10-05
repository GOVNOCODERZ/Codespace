#ifndef MOVIE_H
#define MOVIE_H

/**
 * Сообщения при ошибке программы
 */

#define ERROR_MESSAGE_EMPTY_LIST "ERROR: The list is empty."
#define ERROR_MESSAGE_NOT_A_NUM "ERROR: Inserted text isn't a number."
#define ERROR_MESSAGE_INVALID_MENU_CHOICE "ERROR: Invalid menu option."

#include <iostream>
#include <vector>
using namespace std;

class Movie {
private:
    string name;
    string director;
    int year;
    string genre;
    float rating;
    int length;

public:

    /**
     * Конструкторы класса (по умолчанию; с аргументами; копирование; удаление)
     */

    Movie();
    Movie(string myName, string myDirector, int myYear, string myGenre, float myRating, int myLength);
    Movie(const Movie& other);
    ~Movie();

    friend istream& operator>>(istream& is, Movie& mov);
    friend ostream& operator<<(ostream& os, const Movie& mov);

    /**
     * Геттеры (используются в выборках)
     */

    string getName() const { return name; }
    string getDirector() const { return director; }
    int getYear() const { return year; }
    string getGenre() const { return genre; }
    float getRating() const { return rating; }
    int getLength() const { return length; }


    /**
     * Методы сравнения по рейтингу
     */

    bool operator<(const Movie& other) const { return this->rating < other.rating; }
    bool operator>(const Movie& other) const { return this->rating > other.rating; }

    /**
     * Методы для работы с файлами
     */
    
    static Movie* read_from_file(const string& filename, int& count);
    static void write_to_file(const string& filename, const Movie* movies, int count);

    /**
     * Компаратор для qsort
     */

    int compare_movies_by_rating(const void* a, const void* b);

    /**
     * Выборки
     */

    void selectByDirector(Movie* movies, int count);
    void selectByGenreAndRating(Movie* movies, int count);
    void selectByYears(Movie* movies, int count);

};

#endif