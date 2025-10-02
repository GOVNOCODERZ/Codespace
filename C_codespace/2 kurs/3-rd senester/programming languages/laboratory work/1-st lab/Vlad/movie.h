#ifndef MOVIE_H
#define MOVIE_H

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
    Movie();
    Movie(string myName, string myDirector, int myYear, string myGenre, float myRating, int myLength);
    Movie(const Movie& other);
    ~Movie();

    friend istream& operator>>(istream& is, Movie& mov);
    friend ostream& operator<<(ostream& os, const Movie& mov);

    // Геттеры
    string getName() const { return name; }
    string getDirector() const { return director; }
    int getYear() const { return year; }
    string getGenre() const { return genre; }
    float getRating() const { return rating; }
    int getLength() const { return length; }

    // Сеттеры
    void set_default_Name(string name_to_set) { name = name_to_set; }
    void set_default_Year(int year_to_set) { year = year_to_set; }

    // Операторы сравнения — будем сравнивать по рейтингу
    bool operator<(const Movie& other) const { return this->rating < other.rating; }
    bool operator>(const Movie& other) const { return this->rating > other.rating; }
    bool operator==(const Movie& other) const { return this->rating == other.rating; }
    bool operator<=(const Movie& other) const { return this->rating <= other.rating; }
    bool operator>=(const Movie& other) const { return this->rating >= other.rating; }

    // Статические методы
    static Movie* readFromFile(const string& filename, int& count);
    static void writeToFile(const string& filename, const Movie* movies, int count);
};

#endif