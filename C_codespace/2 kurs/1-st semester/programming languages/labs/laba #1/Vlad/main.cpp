#include "movie.h"
#include <iostream>
using namespace std;

int main() {
    cout << "\nCreating a default empty object:";
    Movie mov0;
    cout << "\nPrinting the object data (via <<):";
    cout << mov0;
    cout << "\nInserting data in that object (via >>):" << endl;
    cin >> mov0;
    cout << "\nPrinting again:";
    cout << mov0;
    
    cout << "\n======================================" << endl;

    cout << "Creating a 1-st object with data." << endl;
    Movie mov1("Bad movie", "Mr. Bad Director", 2010, "Thriller", 3.7, 85);
    cout << mov1;
    cout << "Creating a 2-nd object with data." << endl;
    Movie mov2("Excellent movie", "Mr. Talanted Director", 1992, "Western", 9.2, 140);
    cout << mov2;

    cout << "\n======================================" << endl;

    cout << "\n3-rd object (as 1-st object copy): " << endl;
    Movie mov3(mov1);
    cout << mov3;

    cout << "\n======================================" << endl;

return 0;
}