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

ostream& operator<<(ostream& os, const Movie& mov) {
    os << "Name: \"" << mov.name << "\"" << endl
       << "Director: \"" << mov.director << "\"" << endl
       << "Year: \"" << mov.year << "\"" << endl
       << "Genre: \"" << mov.genre << "\"" << endl
       << "Rating: \"" << mov.rating << "\"" << endl
       << "Length: \"" << mov.length << "\"" << endl;
    return os;
}

Movie* Movie::readFromFile(const string& filename, int& count) {
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

void Movie::writeToFile(const string& filename, const Movie* movies, int count) {
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