#ifndef DIRECT_MERGE_SORT_H
#define DIRECT_MERGE_SORT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstdio>
using namespace std;

struct SortStats {
    long long comparisons = 0;
    long long swaps = 0;
    double time_ms = 0.0;
};

SortStats directMergeSort(const string& filename) {
    auto start_time = chrono::high_resolution_clock::now();
    SortStats stats;

    ifstream initial_file_counter(filename);
    if (!initial_file_counter) {
        cerr << "Failed to open the source file." << endl;
        return stats;
    }

    int n = 0;
    int temp;
    while (initial_file_counter >> temp) {
        n++;
    }
    initial_file_counter.close();

    if (n <= 1) {
        auto end_time = chrono::high_resolution_clock::now();
        stats.time_ms = chrono::duration<double, milli>(end_time - start_time).count();
        return stats;
    }

    string fileA = filename;
    string fileB = "temp_b.txt";
    string fileC = "temp_c.txt";


    for (int run_size = 1; run_size < n; run_size *= 2) {
        ifstream source(fileA);
        ofstream dest_b(fileB);
        ofstream dest_c(fileC);
        
        int current_val;
        int count = 0;
        while (source >> current_val) {
            if ((count / run_size) % 2 == 0) {
                dest_b << current_val << " ";
            } else {
                dest_c << current_val << " ";
            }
            stats.swaps++;
            count++;
        }
        source.close();
        dest_b.close();
        dest_c.close();

        ofstream dest_a(fileA);
        ifstream source_b(fileB);
        ifstream source_c(fileC);

        int val_b, val_c;
        bool has_b = static_cast<bool>(source_b >> val_b);
        bool has_c = static_cast<bool>(source_c >> val_c);
        
        while(has_b || has_c) {
            int b_count = 0;
            int c_count = 0;
            
            while(b_count < run_size && c_count < run_size && has_b && has_c) {
                stats.comparisons++;
                if (val_b <= val_c) {
                    dest_a << val_b << " ";
                    stats.swaps++;
                    b_count++;
                    has_b = static_cast<bool>(source_b >> val_b);
                } else {
                    dest_a << val_c << " ";
                    stats.swaps++;
                    c_count++;
                    has_c = static_cast<bool>(source_c >> val_c);
                }
            }

            while(b_count < run_size && has_b) {
                dest_a << val_b << " ";
                stats.swaps++;
                b_count++;
                has_b = static_cast<bool>(source_b >> val_b);
            }
            while(c_count < run_size && has_c) {
                dest_a << val_c << " ";
                stats.swaps++;
                c_count++;
                has_c = static_cast<bool>(source_c >> val_c);
            }
        }
        
        dest_a.close();
        source_b.close();
        source_c.close();
    }

    remove(fileB.c_str());
    remove(fileC.c_str());

    auto end_time = chrono::high_resolution_clock::now();
    stats.time_ms = chrono::duration<double, milli>(end_time - start_time).count();

    return stats;
}

#endif