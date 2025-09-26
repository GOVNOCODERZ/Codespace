#include "movie.h"
#include <iostream>
#include <vector>
using namespace std;

Movie::Movie(): 
    name(""),
    director(""),
    year(0),
    genre(""),
    rating(0),
    length(0){}

Movie::Movie(string myName, string myDirector, int myYear, string myGenre, float myRating, int myLength): 
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

istream& operator>>(istream& is, Movie& mov) {
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

ostream& operator<<(ostream& os, const Movie& mov) {
    os << "Name: \"" << mov.name << "\"" << endl
       << "Director: \"" << mov.director << "\"" << endl
       << "Year: \"" << mov.year << "\"" << endl
       << "Genre: \"" << mov.genre << "\"" << endl
       << "Rating: \"" << mov.rating << "\"" << endl
       << "Length: \"" << mov.length << "\"" << endl;
    return os;
}