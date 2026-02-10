#include <iostream>
#include <string>

using namespace std;

struct Passenger {
    string fullName;
    int trainNumber;
    string destination;
};

void inputPassenger(Passenger &pas) {
    cout << "Введите ФИО пассажира: ";
    getline(cin, pas.fullName);
    cout << "Введите номер поезда: ";
    cin >> pas.trainNumber;
    cin.ignore();
    
    cout << "Введите станцию назначения: ";
    getline(cin, pas.destination);
}

void outputPassenger(const Passenger &pas) {
    cout << "ФИО: " << pas.fullName << ", Номер поезда: " << pas.trainNumber 
        << ", Станция назначения: " << pas.destination << endl;
}

void inputPassengers(Passenger passengers[], int count) {
    for (int i = 0; i < count; ++i) {
        cout << "\nВведите данные для пассажира " << (i + 1) << ":\n";
        inputPassenger(passengers[i]);
    }
}

void outputPassengers(const Passenger passengers[], int count) {
    for (int i = 0; i < count; ++i) {
        cout << "\nПассажир " << (i + 1) << ":\n";
        outputPassenger(passengers[i]);
    }
}

int main() {
    const int maxPassengers = 100;
    Passenger passengers[maxPassengers];
    int numberOfPassengers;
    bool check = true;
    while(check == true){
        cout<<"1 - Задать структуру.\n2 - Вывести структуру\n0 - Закрыть программу\nВведите действие: ";
        int parameter;
        cin>>parameter;
        switch (parameter){
            case 1:
                cout << "Введите количество пассажиров (максимум " << maxPassengers << "): ";
                cin >> numberOfPassengers;

                if (numberOfPassengers > maxPassengers || numberOfPassengers <= 0) {
                    cout << "Некорректное количество пассажиров." << endl;
                    return 1;
                }

                cin.ignore();

                inputPassengers(passengers, numberOfPassengers);
                break;
            case 2:
                cout << "\nДанные о пассажирах:\n";
                outputPassengers(passengers, numberOfPassengers);
                break;
            case 0:
                check = false;
        }
    }
    return 0;
}