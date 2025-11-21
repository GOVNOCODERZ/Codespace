#include "MemoryAllocator.cpp"
#include <iostream>
#include <vector>
#include <limits>
using namespace std;

int main() {
    constexpr size_t HEAP_SIZE = 65536;
    MemoryAllocator allocator(HEAP_SIZE);
    vector<void*> allocatedBlocks;

    int choice;
    size_t size;

    while (true) {
        cout << "\n===== RAM Management Model =====\n\n";
        cout << "============= MENU =============" << endl;
        cout << "1. Allocate memory block" << endl;
        cout << "2. Deallocate memory block" << endl;
        cout << "3. Display heap status" << endl;
        cout << "0. Exit" << endl;
        cout << "Choose an action: ";

        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "ERROR: Invalid input. Please enter a number from the menu.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                cout << "Enter block size (bytes): ";
                cin >> size;
                if (cin.fail() || size == 0) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "ERROR: Invalid size.\n";
                    break;
                }
                void* ptr = allocator.allocate(size);
                if (ptr) {
                    allocatedBlocks.push_back(ptr);
                    cout << "Allocated at: " << ptr << endl;
                }
                break;
            }

            case 2: {
                if (allocatedBlocks.empty()) {
                    cout << "No allocated blocks available for deallocation.\n";
                    break;
                }
                cout << "Allocated blocks:" << endl;
                for (size_t i = 0; i < allocatedBlocks.size(); ++i) {
                    cout << i << ": " << allocatedBlocks[i] << endl;
                }
                cout << "Enter index to deallocate: ";
                int idx;
                cin >> idx;
                if (cin.fail() || idx < 0 || static_cast<size_t>(idx) >= allocatedBlocks.size()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "ERROR: Invalid index.\n";
                    break;
                }
                allocator.deallocate(allocatedBlocks[idx]);
                allocatedBlocks.erase(allocatedBlocks.begin() + idx);
                cout << "Block deallocated successfully.\n";
                break;
            }

            case 3: {
                allocator.printStatus();
                break;
            }

            case 0: {
                cout << "Exiting the program...\n";
                return 0;
            }

            default: {
                cout << "ERROR: Invalid choice. Please select an option from the menu.\n";
            }
        }
    }
}