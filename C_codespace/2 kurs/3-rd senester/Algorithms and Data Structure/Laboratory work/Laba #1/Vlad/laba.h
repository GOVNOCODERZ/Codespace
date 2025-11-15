#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
using namespace std;

class Timer {
public:
    Timer() : start_time(chrono::high_resolution_clock::now()) {}
    void reset() { start_time = chrono::high_resolution_clock::now(); }
    double elapsed() const {
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
        return static_cast<double>(duration.count()) / 1000.0;
    }
private:
    chrono::high_resolution_clock::time_point start_time;
};

template<typename T>
class Array {

private:
    T* data;
    int size;

    void heapify(long start, long end, long& comparisons, long& swaps) {
        long root = start;
        long largest = root;
        long left = 2 * root + 1;
        long right = 2 * root + 2;

        if (left < end) {
            comparisons++;
            if (data[left] > data[largest]) {
                largest = left;
            }
        }
        if (right < end) {
            comparisons++;
            if (data[right] > data[largest]) {
                largest = right;
            }
        }

        if (largest != root) {
            swap(data[root], data[largest]);
            swaps++;
            heapify(largest, end, comparisons, swaps);
        }
    };

public:
    Array() : data(nullptr), size(0) {}
    Array(long s) : size(s) {
        if (size > 0) {
            data = new T[size];
        } else {
            data = nullptr;
        }
    }
    Array(const Array& other) : size(other.size) {
        if (size > 0) {
            data = new T[size];
            for (long i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        } else {
            data = nullptr;
        }
    }
    ~Array() { delete[] data; }

    void fillFromKeyboard() {
        cout << "Enter " << size << " elements of array:\n";
        for (int i = 0; i < size; ++i) {
            cout << "Element #" << i << ": ";
            cin >> data[i];
        }
    }

    void fillRandom(T min_val, T max_val) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<T> dis(min_val, max_val);
        for (long i = 0; i < size; ++i) {
            data[i] = dis(gen);
        }
    }

    void fillSorted() {
        for (long i = 0; i < size; ++i) {
            data[i] = static_cast<T>(i);
        }
    }

    void fillReverseSorted() {
        for (long i = 0; i < size; ++i) {
            data[i] = static_cast<T>(size - i - 1);
        }
    }

    void display() const {
        cout << "First 15 elements of array: ";
        for (int i = 0; i < min(size, 15); ++i) {
            cout << data[i] << " ";
        }
        if (size > 15) {
            cout << "...";
        }
        cout << endl;
    }

    void saveToFile(const string& filename) const {
        ofstream file(filename);
        if (file.is_open()) {
            print(file);
            file.close();
        } else {
            cerr << "Error opening file " << filename << " for writing.\n";
        }
    }

    void print(ostream& os) const {
        for (long i = 0; i < size; ++i) {
            os << data[i] << " ";
        }
        os << "\n";
    }

    void selectSort(long& comparisons, long& swaps) {
        comparisons = 0;
        swaps = 0;
        for (long i = 0; i < size - 1; ++i) {
            long min_idx = i;
            for (long j = i + 1; j < size; ++j) {
                comparisons++;
                if (data[j] < data[min_idx]) {
                    min_idx = j;
                }
            }
            if (min_idx != i) {
                swap(data[i], data[min_idx]);
                swaps++;
            }
        }
    }

    void heapSort(long& comparisons, long& swaps) {
        comparisons = 0;
        swaps = 0;
        if (size <= 1) return;

        // Phase 1: Build heap
        for (long i = size / 2 - 1; i >= 0; --i) {
            heapify(i, size, comparisons, swaps);
        }

        // Phase 2: Extract elements from heap
        for (long i = size - 1; i > 0; --i) {
            swap(data[0], data[i]);
            swaps++;
            heapify(0, i, comparisons, swaps);
        }
    }

};

void runSingleSort() {
    long n;
    int min_val, max_val;
    int sort_choice;

    cout << "Enter the array size: ";
    cin >> n;
    cout << "Enter the minimum and maximum value for elements: ";
    cin >> min_val >> max_val;

    Array<int> initial_arr(n);
    initial_arr.fillRandom(min_val, max_val);

    cout << "\nInitial array:\n";
    initial_arr.display();

    Array<int> sorted_arr = initial_arr;

    cout << "Choose the sorting method:\n";
    cout << "1. Selection Sort\n";
    cout << "2. Heap Sort\n";
    cin >> sort_choice;

    long comparisons = 0, swaps = 0;
    Timer timer;

    switch (sort_choice) {
        case 1:
            timer.reset();
            sorted_arr.selectSort(comparisons, swaps);
            break;
        case 2:
            timer.reset();
            sorted_arr.heapSort(comparisons, swaps);
            break;
        default:
            cout << "Invalid choice.\n";
            return;
    }

    double elapsed_time = timer.elapsed();

    cout << "\nSorted array:\n";
    sorted_arr.display();

    cout << "Sorting time: " << elapsed_time << " ms\n";
    cout << "Number of comparisons: " << comparisons << "\n";
    cout << "Number of swaps: " << swaps << "\n";

    string initial_filename = "initial_";
    string sorted_filename = "sorted_";
    switch (sort_choice) {
        case 1: 
            initial_filename += "select.txt";
            sorted_filename += "select.txt";
            break;
        case 2: 
            initial_filename += "heap.txt";
            sorted_filename += "heap.txt";
            break;
    }

    initial_arr.saveToFile(initial_filename);
    cout << "Initial array saved to " << initial_filename << "\n";
    
    sorted_arr.saveToFile(sorted_filename);
    cout << "Sorted array saved to " << sorted_filename << "\n";
};

void runStatisticalAnalysis() {
    int fill_choice;
    long min_size, max_size, step;
    int sort_choice;

    cout << "Choose the array generation method:\n";
    cout << "1. Random values\n";
    cout << "2. Sorted sequence\n";
    cout << "3. Reverse sorted sequence\n";
    cin >> fill_choice;

    cout << "Enter minimum size, maximum size, and step: ";
    cin >> min_size >> max_size >> step;

    cout << "Choose the sorting method:\n";
    cout << "1. Selection Sort\n";
    cout << "2. Heap Sort\n";
    cin >> sort_choice;

    string filename = "timing_data_";
    switch (fill_choice) {
        case 1: filename += "random_"; break;
        case 2: filename += "sorted_"; break;
        case 3: filename += "reverse_"; break;
    }
    switch (sort_choice) {
        case 1: filename += "select.txt"; break;
        case 2: filename += "heap.txt"; break;
    }

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file " << filename << " for writing.\n";
        return;
    }

    cout << "\nProcessing data...\n";

    file << "# Size\tTime (ms)\n";

    for (long size = min_size; size <= max_size; size += step) {
        Array<int> arr(size);
        switch (fill_choice) {
            case 1: arr.fillRandom(0, 10000); break;
            case 2: arr.fillSorted(); break;
            case 3: arr.fillReverseSorted(); break;
        }

        Timer timer;
        long comparisons, swaps;

        switch (sort_choice) {
            case 1:
                timer.reset();
                arr.selectSort(comparisons, swaps);
                break;
            case 2:
                timer.reset();
                arr.heapSort(comparisons, swaps);
                break;
        }

        double elapsed_time = timer.elapsed();
        file << size << "\t" << elapsed_time << "\n";
    }

    file.close();
    cout << "Timing data saved to " << filename << "\n";
};

#endif