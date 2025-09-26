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


    string getName(){ 
        return name;
    }

    int getYear(){ 
        return year;
    }

    void set_default_Name(string name_to_set){ 
        name = name_to_set;
    }

    void set_default_Year(int year_to_set){ 
        year = year_to_set;
    }
};
#endif