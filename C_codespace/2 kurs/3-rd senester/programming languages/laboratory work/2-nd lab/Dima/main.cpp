#include "storage.h"

void menu() {
    cout << "\n=== Меню программы ===" << endl;
    cout << "1. Добавить жёсткий диск (HDD)" << endl;
    cout << "2. Добавить флэш-накопитель" << endl;
    cout << "3. Вывести список устройств" << endl;
    cout << "4. Фильтр по ёмкости" << endl;
    cout << "5. Показать статистику (ёмкость/цена)" << endl;
    cout << "6. Сортировка по ёмкости" << endl;
    cout << "7. Сохранить в файл (data_raw.txt)" << endl;
    cout << "8. Загрузить из файла (data_raw.txt)" << endl;
    cout << "0. Выход" << endl;
    cout << "Выбор: ";
}

int main() {
    // Создаем объект класса-контейнера Ms (Shop)
    Shop myShop;
    int choice;

    // Предварительно заполним парой объектов
    myShop.addDevice(new HDD("Seagate", "BarraCuda", 4500, 1000, 7200));
    myShop.addDevice(new FlashD("Kingston", "DataTraveler", 600, 64, 150));

    do {
        menu();
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "--- Добавление жёсткого диска (HDD) ---" << endl;
            HDD* h = new HDD();
            cin >> *h; // Используем перегрузку >>
            myShop.addDevice(h);
            break;
        }
        case 2: {
            cout << "--- Добавление флэш-накопителя ---" << endl;
            FlashD* f = new FlashD();
            cin >> *f; // Используем перегрузку >>
            myShop.addDevice(f);
            break;
        }
        case 3:
            myShop.showAll();
            break;
        case 4: {
            int min, max;
            cout << "Минимальная ёмкость: "; cin >> min;
            cout << "Максимальная ёмкость : "; cin >> max;
            myShop.showByCapacityRange(min, max);
            break;
        }
        case 5:
            myShop.showStats();
            break;
        case 6:
            myShop.sortByCapacity();
            myShop.showAll();
            break;
        case 7:
            myShop.saveToFile("data_raw.txt");
            break;
        case 8:
            myShop.loadFromFile("data_raw.txt");
            break;
        case 0:
            cout << "Выход из программы." << endl;
            break;
        default:
            cout << "Неправильный выбор." << endl;
        }
    } while (choice != 0);

    return 0;
}