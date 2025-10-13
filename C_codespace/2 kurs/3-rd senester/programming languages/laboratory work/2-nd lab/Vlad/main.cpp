#include "payment.h"
#include "payment.cpp"
#include <iostream>
using namespace std;

const string INPUT_FILENAME = "input.txt";
const string OUTPUT_FILENAME = "output.txt";


int main() {
    Sales sales;
    bool running = true;

    while (running) {
        cout << "\n======================================" << endl;
        cout << "           PAYMENT SYSTEM MENU" << endl;
        cout << "======================================" << endl;
        cout << "1. Add Bank Transfer" << endl;
        cout << "2. Add WebMoney" << endl;
        cout << "3. Display All Payments" << endl;
        cout << "4. Select by Date" << endl;
        cout << "5. Select by Amount Range" << endl;
        cout << "6. Show Statistics by Type" << endl;
        cout << "7. Load from File" << endl;
        cout << "8. Save to File" << endl;
        cout << "9. Exit" << endl;
        cout << "Choose an action: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                string date, bank;
                float amount;
                cout << "Enter date: ";
                cin >> date;
                cout << "Enter amount: ";
                cin >> amount;
                cout << "Enter bank: ";
                cin >> bank;
                sales.addPayment(new BankTransfer(date, amount, bank));
                break;
            }
            case 2: {
                string date;
                float amount, commission;
                cout << "Enter date: ";
                cin >> date;
                cout << "Enter amount: ";
                cin >> amount;
                cout << "Enter commission: ";
                cin >> commission;
                sales.addPayment(new WebMoney(date, amount, commission));
                break;
            }
            case 3:
                sales.displayAll();
                break;
            case 4: {
                string date;
                cout << "Enter date: ";
                cin >> date;
                sales.selectByDate(date);
                break;
            }
            case 5: {
                float min, max;
                cout << "Enter min amount: ";
                cin >> min;
                cout << "Enter max amount: ";
                cin >> max;
                sales.selectByAmountRange(min, max);
                break;
            }
            case 6:
                sales.showStatsByType();
                break;
            case 7: {
                sales.loadFromFile(INPUT_FILENAME);
                break;
            }
            case 8: {
                sales.saveToFile(OUTPUT_FILENAME);
                break;
            }
            case 9:
                running = false;
                break;
            default:
                cout << "Invalid choice. Try again." << endl;
        }
    }

    return 0;
}