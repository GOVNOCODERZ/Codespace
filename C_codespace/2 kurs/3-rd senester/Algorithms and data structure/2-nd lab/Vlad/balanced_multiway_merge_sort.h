#ifndef BALANCED_MULTIWAY_MERGE_SORT_H
#define BALANCED_MULTIWAY_MERGE_SORT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <queue>
#include <functional>
#include "direct_merge_sort.h"
using namespace std;

SortStats balancedMultiwayMergeSort(const string& filename, int m) {
    if (m < 2) {
        cerr << "Number of ways (m) must be 2 or greater." << endl;
        return {};
    }

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

    vector<string> files1, files2;
    for (int i = 0; i < m; ++i) {
        files1.push_back("temp_1_" + to_string(i) + ".txt");
        files2.push_back("temp_2_" + to_string(i) + ".txt");
    }

    {
        ifstream source(filename);
        vector<ofstream> dests;
        for (int i = 0; i < m; ++i) {
            dests.emplace_back(files1[i]);
        }
        int file_index = 0;
        while (source >> temp) {
            dests[file_index] << temp << " ";
            stats.swaps++;
            file_index = (file_index + 1) % m;
        }
    }

    long long run_size = 1;
    vector<string>* from_files = &files1;
    vector<string>* to_files = &files2;

    while (run_size < n) {
        vector<ifstream> sources;
        for (int i = 0; i < m; ++i) {
            sources.emplace_back((*from_files)[i]);
        }

        vector<ofstream> dests;
        for (int i = 0; i < m; ++i) {
            dests.emplace_back((*to_files)[i]);
        }

        int dest_index = 0;
        while (true) {
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
            
            vector<int> current_run_counts(m, 0);

            for (int i = 0; i < m; ++i) {
                if (sources[i] >> temp) {
                    pq.push({temp, i});
                }
            }

            if (pq.empty()) {
                break;
            }

            while (!pq.empty()) {
                pair<int, int> min_pair = pq.top();
                pq.pop();

                int value = min_pair.first;
                int file_idx = min_pair.second;

                dests[dest_index] << value << " ";
                stats.swaps++;

                current_run_counts[file_idx]++;

                if (current_run_counts[file_idx] < run_size) {
                    if (sources[file_idx] >> temp) {
                        stats.comparisons++;
                        pq.push({temp, file_idx});
                    }
                }
            }
            dest_index = (dest_index + 1) % m;
        }
        
        run_size *= m;
        swap(from_files, to_files);
    }

    {
        ofstream final_dest(filename);
        ifstream final_source((*from_files)[0]);
        while (final_source >> temp) {
            final_dest << temp << " ";
        }
    }

    for (int i = 0; i < m; ++i) {
        remove(files1[i].c_str());
        remove(files2[i].c_str());
    }

    auto end_time = chrono::high_resolution_clock::now();
    stats.time_ms = chrono::duration<double, milli>(end_time - start_time).count();

    return stats;
}

#endif