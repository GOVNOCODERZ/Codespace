#include "MemoryAllocator.h"
#include "MemoryAllocator.cpp"
#include <iostream>
#include <vector>
#include <limits>
using namespace std;

int main() {
    constexpr size_t HEAP_SIZE = 65536; // 64 KB
    MemoryAllocator allocator(HEAP_SIZE);
    vector<void*> allocatedBlocks; // Хранение адресов выделенных блоков

    int choice;
    size_t size;
    void* ptr;

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
            case 1: // Allocate memory
                cout << "Enter block size (bytes): ";
                cin >> size;
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "ERROR: Invalid size. Please enter a positive integer.\n";
                    break;
                }
                ptr = allocator.allocate(size);
                if (ptr) {
                    allocatedBlocks.push_back(ptr);
                    cout << "Allocated at: " << ptr << endl;
                }
                break;

            case 2: // Deallocate memory
                if (allocatedBlocks.empty()) {
                    cout << "No allocated blocks available for deallocation.\n";
                    break;
                }
                cout << "Allocated blocks:" << endl;
                for (size_t i = 0; i < allocatedBlocks.size(); ++i) {
                    cout << i << ": " << allocatedBlocks[i] << endl;
                }
                cout << "Enter index to deallocate: ";
                int index;
                cin >> index;
                if (cin.fail() || index < 0 || static_cast<size_t>(index) >= allocatedBlocks.size()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "ERROR: Invalid index.\n";
                    break;
                }
                allocator.deallocate(allocatedBlocks[index]);
                allocatedBlocks.erase(allocatedBlocks.begin() + index);
                cout << "Block deallocated successfully.\n";
                break;

            case 3: // Display heap status
                allocator.printStatus();
                break;

            case 0: // Exit
                cout << "Exiting the program...\n";
                return 0;

            default:
                cout << "ERROR: Invalid choice. Please select an option from the menu.\n";
        }
    }

    return 0;
}