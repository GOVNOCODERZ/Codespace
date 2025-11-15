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

bool findPassengers(const Passenger passengers[], int count, int trainNumber, const string &destination) {
    bool found = false;
    cout << "\nНайденные пассажиры на поезде " << trainNumber << " до станции " << destination << ":\n";
    
    for (int i = 0; i < count; ++i) {
        if (passengers[i].trainNumber == trainNumber && passengers[i].destination == destination) {
            outputPassenger(passengers[i]);
            found = true;
        }
    }
    
    return found;
}

int countPassengers(const Passenger passengers[], int count, int trainNumber, const string &destination) {
    int countFound = 0;
    
    for (int i = 0; i < count; ++i) {
        if (passengers[i].trainNumber == trainNumber && passengers[i].destination == destination) {
            countFound++;
        }
    }
    
    return countFound;
}

int main() {
    const int maxPassengers = 100;
    Passenger passengers[maxPassengers];
    int numberOfPassengers;
    bool check = true;
    while (check == true){
        cout<<"1 - Задать структуру.\n2 - Вывести структуру\n3 - Поиск количества пассажиров заданного вагона, едущего до заданной станции\n0 - Закрыть программу\nВведите действие: ";
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
            case 3:{
                int searchTrainNumber;
                string searchDestination;

                cout << "\nВведите номер поезда для поиска: ";
                cin >> searchTrainNumber;
                cin.ignore();

                cout << "Введите станцию назначения для поиска: ";
                getline(cin, searchDestination);

                if (!findPassengers(passengers, numberOfPassengers, searchTrainNumber, searchDestination)) {
                    cout << "Пассажиры не найдены." << endl;
                }

                int count = countPassengers(passengers, numberOfPassengers, searchTrainNumber, searchDestination);
                cout << "\nКоличество пассажиров на поезде " << searchTrainNumber 
                    << " до станции " << searchDestination << ": " << count << endl;
                break;}
            case 0:
                check = false;
        }
    }
return 0;
}