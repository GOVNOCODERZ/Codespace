#include "movie.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdlib>
using namespace std;

// Prototypes for selection functions
void selectByDirector(Movie* movies, int count);
void selectByGenreAndRating(Movie* movies, int count);
void selectByYears(Movie* movies, int count);

// Comparator prototype for qsort
int compareMoviesByRating(const void* a, const void* b);

// Function to add a new movie
void addMovie(Movie*& movies, int& count) {
    cout << "\n=== ADDING A NEW MOVIE ===" << endl;
    Movie newMovie;
    cin >> newMovie;

    Movie* newMovies = new Movie[count + 1];
    for (int i = 0; i < count; ++i) {
        newMovies[i] = movies[i];
    }
    newMovies[count] = newMovie;

    if (movies != nullptr) {
        delete[] movies;
    }

    movies = newMovies;
    count++;
    cout << "Movie added successfully!" << endl;
}

int main() {
    Movie* movies = nullptr;
    int movieCount = 0;
    bool running = true;

    while (running) {
        cout << "\n======================================" << endl;
        cout << "           MAIN MENU" << endl;
        cout << "======================================" << endl;
        cout << "1. Load data from file" << endl;
        cout << "2. Display data on screen" << endl;
        cout << "3. Save data to file" << endl;
        cout << "4. Select movies by director" << endl;
        cout << "5. Select movies by genre and rating" << endl;
        cout << "6. Select movies by years" << endl;
        cout << "7. Sort movies by rating (qsort)" << endl;
        cout << "8. Add new movie" << endl;
        cout << "9. Exit" << endl;
        cout << "\nChoose an action: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                ifstream inFile("movies.txt");
                if (inFile.is_open()) {
                    if (movies != nullptr) {
                        delete[] movies;
                        movies = nullptr;
                        movieCount = 0;
                    }
                    movies = Movie::readFromFile(inFile, movieCount);
                    inFile.close();
                    if (movies != nullptr) {
                        cout << "Data loaded successfully. Number of movies: " << movieCount << endl;
                    } else {
                        cout << "Error loading data." << endl;
                    }
                } else {
                    cout << "Could not open file movies.txt" << endl;
                }
                break;
            }

            case 2: {
                if (movies == nullptr || movieCount == 0) {
                    cout << "No data to display." << endl;
                } else {
                    cout << "\n=== ALL MOVIES ===" << endl;
                    for (int i = 0; i < movieCount; ++i) {
                        cout << movies[i] << endl;
                    }
                }
                break;
            }

            case 3: {
                if (movies == nullptr || movieCount == 0) {
                    cout << "No data to save." << endl;
                } else {
                    ofstream outFile("output.txt");
                    if (outFile.is_open()) {
                        Movie::writeToFile(outFile, movies, movieCount);
                        outFile.close();
                        cout << "Data saved to output.txt" << endl;
                    } else {
                        cout << "Could not open file for writing." << endl;
                    }
                }
                break;
            }

            case 4:
                selectByDirector(movies, movieCount);
                break;

            case 5:
                selectByGenreAndRating(movies, movieCount);
                break;

            case 6:
                selectByYears(movies, movieCount);
                break;

            case 7: {
                if (movies == nullptr || movieCount == 0) {
                    cout << "No data to sort." << endl;
                } else {
                    cout << "Sorting by rating (descending) using qsort..." << endl;
                    qsort(movies, movieCount, sizeof(Movie), compareMoviesByRating);
                    cout << "Sorting completed." << endl;
                }
                break;
            }

            case 8: {
                addMovie(movies, movieCount);
                break;
            }

            case 9: {
                cout << "Exiting the program..." << endl;
                running = false;
                break;
            }

            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }

    // Free memory before exit
    if (movies != nullptr) {
        delete[] movies;
    }

    return 0;
}

// Comparator for qsort
int compareMoviesByRating(const void* a, const void* b) {
    const Movie* movieA = (const Movie*)a;
    const Movie* movieB = (const Movie*)b;

    if (movieA->getRating() > movieB->getRating()) return -1;  // descending order
    else if (movieA->getRating() < movieB->getRating()) return 1;
    else return 0;
}

// Selection functions
void selectByDirector(Movie* movies, int count) {
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

void selectByGenreAndRating(Movie* movies, int count) {
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

void selectByYears(Movie* movies, int count) {
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