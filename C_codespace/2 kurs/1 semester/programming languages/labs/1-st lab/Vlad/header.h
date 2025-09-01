#ifndef Movie_H
#define Movie_H

#include <iostream>
#include <stdexcept>
using namespace std;

class Movie {
private:

    string name;
    string director; 
    int year;
    string genre;
    int rating;
    int time;
    
    void copyFrom(const Movie& other);     

public:
    Movie(); // по умолчанию                               
    Movie(string myName, string myDirector, int myYear, string myGenre, int myrating, int myTime); // с параметрами  
        name(myName), 
        director(myDirector), 
        year(myYear), 
        genre(myGenre),
        rating(myrating),
        time(myTime)                     
    Movie(const Movie& other); // копирование
    ~Movie(); // удаление                              

    void input();                          
    void output() const;                   

    friend std::istream& operator>>(std::istream& is, Movie& arr);
    friend std::ostream& operator<<(std::ostream& os, const Movie& arr);
};

#endif