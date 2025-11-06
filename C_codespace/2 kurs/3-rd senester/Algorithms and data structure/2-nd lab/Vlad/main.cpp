#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <clocale>
#include <iomanip>
#include "direct_merge_sort.h"
#include "balanced_multiway_merge_sort.h" 
using namespace std;

enum DataGenerationMode {
    RANDOM,
    SORTED,
    REVERSE_SORTED
};

void generateDataFile(const string& filename, int size, int min_val, int max_val, DataGenerationMode mode) {
    ofstream file(filename);
    if (!file) {
        cerr << "Failed to create file for writing data." << endl;
        return;
    }

    vector<int> data(size);
    if (mode == RANDOM) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distrib(min_val, max_val);
        for (int i = 0; i < size; ++i) {
            data[i] = distrib(gen);
        }
    } else {
        for (int i = 0; i < size; ++i) {
            data[i] = i + min_val;
        }
        if (mode == REVERSE_SORTED) {
            reverse(data.begin(), data.end());
        }
    }

    for (int i = 0; i < size; ++i) {
        file << data[i] << (i == size - 1 ? "" : " ");
    }
    file.close();
}

void singleSortMode() {
    int size, min_val, max_val, method;
    string filename = "data.txt";

    cout << "Enter the size of the numeric sequence: ";
    cin >> size;
    cout << "Enter the minimum value of an element: ";
    cin >> min_val;
    cout << "Enter the maximum value of an element: ";
    cin >> max_val;

    generateDataFile(filename, size, min_val, max_val, RANDOM);
    cout << "File '" << filename << "' with random data has been generated." << endl;

    cout << "\nChoose the external sorting method:\n";
    cout << "1. Direct merge sort\n";
    cout << "2. Balanced multiway merge sort\n";
    cout << "Your choice: ";
    cin >> method;

    SortStats stats;
    if (method == 1) {
        stats = directMergeSort(filename);
    } else if (method == 2) {
        int m;
        cout << "Enter the number of ways (m) for merging (e.g., 3): ";
        cin >> m;
        stats = balancedMultiwayMergeSort(filename, m);
    } else {
        cout << "Invalid choice." << endl;
        return;
    }

    cout << "\nSorting results:\n";
    cout << "Execution time: " << stats.time_ms << " ms\n";
    cout << "Number of comparisons: " << stats.comparisons << "\n";
    cout << "Number of swaps (writes): " << stats.swaps << "\n";
    cout << "Sorted data is in the file '" << filename << "'." << endl;
}

void statisticsMode() {
    int data_mode_choice, sort_method_choice;
    int start_size, end_size, step;
    string output_filename;

    cout << "Choose the method of forming the sequence elements:\n";
    cout << "1. Random values\n";
    cout << "2. Sorted sequence\n";
    cout << "3. Values in reverse order\n";
    cout << "Your choice: ";
    cin >> data_mode_choice;

    DataGenerationMode data_mode;
    switch (data_mode_choice) {
        case 1: data_mode = RANDOM; break;
        case 2: data_mode = SORTED; break;
        case 3: data_mode = REVERSE_SORTED; break;
        default: cout << "Invalid choice.\n"; return;
    }

    cout << "Set the range of sequence size change:\n";
    cout << "Start size: ";
    cin >> start_size;
    cout << "End size: ";
    cin >> end_size;
    cout << "Step size: ";
    cin >> step;

    cout << "\nChoose the external sorting method:\n";
    cout << "1. Direct merge sort\n";
    cout << "2. Balanced multiway merge sort\n";
    cout << "Your choice: ";
    cin >> sort_method_choice;
    
    int m = 0;
    if(sort_method_choice == 2) {
        cout << "Enter the number of ways (m) for merging (e.g., 3): ";
        cin >> m;
    }

    cout << "Enter the filename to save results (e.g., stats.csv): ";
    cin >> output_filename;

    ofstream stats_file(output_filename);
    if (!stats_file) {
        cerr << "Failed to create statistics file." << endl;
        return;
    }

    stats_file << "Size Time_(ms)\n";
    stats_file << fixed << setprecision(0);
    cout << "\nStarting statistics collection...\n";

    for (int size = start_size; size <= end_size; size += step) {
        cout << "Sorting for size " << size << "..." << endl;
        string temp_filename = "temp_data.txt";
        generateDataFile(temp_filename, size, 0, size * 10, data_mode);

        SortStats stats;
        if (sort_method_choice == 1) {
            stats = directMergeSort(temp_filename);
        } else if (sort_method_choice == 2) {
            stats = balancedMultiwayMergeSort(temp_filename, m);
        } else {
            cout << "Invalid sorting method choice." << endl;
            return;
        }

        stats_file << size << " " << stats.time_ms << "\n";
    }

    stats_file.close();
    cout << "\nStatistics collection finished. Results saved to file '" << output_filename << "'." << endl;
}


int main() {
    int choice;
    do {
        cout << "\n--- Menu ---\n";
        cout << "1. Sort a data file (random values)\n";
        cout << "2. Statistics accumulation mode\n";
        cout << "0. Exit\n";
        cout << "Your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                singleSortMode();
                break;
            case 2:
                statisticsMode();
                break;
            case 0:
                cout << "Exiting the program." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);

    return 0;
}