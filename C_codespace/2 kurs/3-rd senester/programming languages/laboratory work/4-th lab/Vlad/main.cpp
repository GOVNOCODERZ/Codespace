#include "TextProcessor.h"
#include <iostream>
using namespace std;

int main() {
    TextProcessor processor;
    bool running = true;

    while (running) {
        cout << "\n======================================" << endl;
        cout << "          TEXT PROCESSOR MENU" << endl;
        cout << "======================================" << endl;
        cout << "1. Load text from file" << endl;
        cout << "2. Save text to file" << endl;
        cout << "3. Enter text manually" << endl;
        cout << "4. Display original text" << endl;
        cout << "5. Display sentences" << endl;
        cout << "6. Find most frequent letter in each sentence" << endl;
        cout << "0. Exit" << endl;
        cout << "Choose an action: ";

        int choice;
        cin >> choice;
        cin.ignore(); // Очищаем буфер после ввода числа

        switch (choice) {
            case 1: {
                string filename;
                cout << "Enter filename to load from: ";
                getline(cin, filename);
                processor.loadFromFile(filename);
                break;
            }
            case 2: {
                string filename;
                cout << "Enter filename to save to: ";
                getline(cin, filename);
                processor.saveToFile(filename);
                break;
            }
            case 3: {
                cout << "Enter text (end with an empty line):" << endl;
                string line, full_text;
                while (getline(cin, line) && !line.empty()) {
                    full_text += line + " ";
                }
                processor.setText(full_text);
                cout << "Text entered." << endl;
                break;
            }
            case 4: {
                processor.displayText();
                break;
            }
            case 5: {
                processor.displaySentences();
                break;
            }
            case 6: {
                processor.findMostFrequentLetterInEachSentence();
                break;
            }
            case 0: {
                running = false;
                cout << "Exiting..." << endl;
                break;
            }
            default: {
                cout << "Invalid choice. Please try again." << endl;
            }
        }
    }

    return 0;
}