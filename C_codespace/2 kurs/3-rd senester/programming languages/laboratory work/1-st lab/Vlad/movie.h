#ifndef MOVIE_H
#define MOVIE_H

/**
 * Сообщения при ошибке программы
 */

#define ERROR_MESSAGE_EMPTY_LIST "ERROR: The list is empty."
#define ERROR_MESSAGE_NOT_A_NUM "ERROR: Inserted text isn't a number."
#define ERROR_MESSAGE_INVALID_MENU_CHOICE "ERROR: Invalid menu option."
#define ERROR_MESSAGE_FILE_OPENING_FAIL "ERROR: Failed to open file."
#define ERROR_MESSAGE_FILE_READING_FAIL "ERROR: Failed to read from file."
#define ERROR_MESSAGE_FILE_WRITING_FAIL "ERROR: Failed to write to file."

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

    // Методы для ввода/вывода
    void read(istream& is);
    void print(ostream& os) const;

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
    
    static vector<Movie> read_from_file(const string& filename);
    static void write_to_file(const string& filename, const vector<Movie>& movies);

    /**
     * Выборки
     */

    static void select_by_director(vector<Movie>& movies, int count);
    static void select_by_genre_and_rating(vector<Movie>& movies, int count);
    static void select_by_years(vector<Movie>& movies, int count);

    /**
     * Сортировка фильмов по рейтингу
     */

    static void sort_by_rating(vector<Movie>& movies);

};

#endif