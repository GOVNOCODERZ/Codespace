#include <iostream>
using namespace std;

struct Info {
    int d;
};

struct Node {
    Info info;
    Node* next;
    Node* prev;
};

Node* start(void);
Node* add(Node* pend);
void printForw(Node* pbegin);
void printBack(Node* pend);

int main(void) {
    Node *pbegin, *pend;
    bool check = true;
    while (check == true){
        cout<<"1 - Задать список\n2 - Добавить элемент в список\n3 - Вывести список\n0 - Закрыть программу\nВведите действие: ";
        int parameter;
        cin>>parameter;
        switch (parameter){
            case 1:
            pend = pbegin = start();
            cout<<"Вы задали список\n";
                break;
            case 2:
            cout <<"Введтие количество элементов, которое хотите занести в список: ";
            int count;
            cin>>count;
            for(count; count != 0; count--) {
                pend = add(pend);
            }
                break;
            case 3:{
                int par2;
                cout<<"Как вы хотите вывести список?\n1 - От начального элемента до конечного\n2 - От конечного элемента до начального\nВведите параметр: ";
                cin>>par2;
            switch (par2){
                case 1:
                    cout << "\nСписок от начала к концу:\n";
                    printForw(pbegin);
                    break;
                case 2:
                    cout << "\nСписок от конца к началу:\n";
                    printBack(pend);
                    break;
            }
                break;}
            case 0:
                check = false;
        }
    }

    
    return 0;
}

Node* start(void) {
    Node* pv = new Node;
    cout << "Введите число: ";
    cin >> pv->info.d;
    pv->next = nullptr;
    pv->prev = nullptr; 
    return pv;
}

Node* add(Node* pend) {
    Node* pv = new Node;
    cout << "Введите число: ";
    cin >> pv->info.d;
    pv->next = nullptr;
    pv->prev = pend;
    pend->next = pv;
    return pv;
}

void printForw(Node* pbegin) {
    Node* pv = pbegin;
    while (pv) {
        cout << pv->info.d << " ";
        pv = pv->next;
    }
    cout << endl;
}

void printBack(Node* pend) {
    Node* pv = pend;
    while (pv) {
        cout << pv->info.d << " ";
        pv = pv->prev;
    }
    cout << endl;
}