#include "BinarySearchTree.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    // Используем AVL<double> согласно варианту с вещественными числами
    AVL<double> tree;
    int choice;
    double val;
    string filename;

    // Для теста операторов
    AVL<double> testTree; 

    while (true) {
        cout << "\n============== MENU ==============\n";
        cout << "1. Add node (keyboard input)\n";
        cout << "2. Remove node\n";
        cout << "3. Search node\n";
        cout << "4. Display tree (visual)\n";
        cout << "5. Tree traversals (PreOrder, InOrder, PostOrder)\n";
        cout << "6. Execute Variant 5 Task\n";
        cout << "7. Save to file\n";
        cout << "8. Load from file\n";
        cout << "9. Test operators (= and ==)\n";
        cout << "0. Exit\n";
        cout << "==================================\n";
        cout << "Your choice: ";
        
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
        case 1:
            cout << "Enter value (double): ";
            cin >> val;
            tree.addValue(val);
            cout << "Node added.\n";
            break;
        case 2:
            cout << "Enter value to remove: ";
            cin >> val;
            tree.removeValue(val);
            cout << "Remove command executed.\n";
            break;
        case 3:
            cout << "Enter value to search: ";
            cin >> val;
            if (tree.searchValue(val))
                cout << "Node FOUND.\n";
            else
                cout << "Node NOT found.\n";
            break;
        case 4:
            cout << "\n--- Current Tree (operator<<) ---\n";
            cout << tree; // Использование перегруженного оператора <<
            break;
        case 5:
            cout << "PreOrder traversal: "; tree.preOrder();
            cout << "InOrder traversal: "; tree.inOrder();
            cout << "PostOrder traversal: "; tree.postOrder();
            break;
        case 6:
            tree.executeVariantTask(); // Выполняем задачу варианта 5
            break;
        case 7:
            cout << "Enter filename: ";
            cin >> filename;
            tree.saveToFile(filename);
            break;
        case 8:
            cout << "Enter filename: ";
            cin >> filename;
            tree.loadFromFile(filename);
            break;
        case 9:
            cout << "\n--- Testing Operators ---\n";
            testTree = tree; // Тест operator=
            cout << "Tree copied to testTree via operator=.\n";
            cout << "Comparison tree == testTree: " << (tree == testTree ? "EQUAL" : "NOT EQUAL") << endl;
            
            cout << "Adding 999.9 to testTree...\n";
            testTree.addValue(999.9);
            cout << "Comparison tree == testTree: " << (tree == testTree ? "EQUAL" : "NOT EQUAL") << endl;
            break;
        case 0:
            return 0;
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}