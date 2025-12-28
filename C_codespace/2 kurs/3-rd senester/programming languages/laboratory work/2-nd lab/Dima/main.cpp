#include "storage.h"

void showMenu() {
    cout << "\n1. Добавить HDD\n2. Добавить Flash\n3. Показать все\n"
         << "4. Фильтр емкости\n5. Статистика\n6. Сортировка\n"
         << "7. Сохранить\n8. Загрузить\n0. Выход\nВыбор: ";
}

int main() {
    Shop shop;
    int choice;

    do {
        showMenu();
        if (!(cin >> choice)) break;

        switch (choice) {
            case 1: {
                HDD* h = new HDD();
                cin >> *h;
                shop.addDevice(h);
                break;
            }
            case 2: {
                FlashD* f = new FlashD();
                cin >> *f;
                shop.addDevice(f);
                break;
            }
            case 3: shop.showAll(); break;
            case 4: {
                int a, b;
                cout << "Мин и макс: "; cin >> a >> b;
                shop.showByCapacityRange(a, b);
                break;
            }
            case 5: shop.showStats(); break;
            case 6: shop.sortByCapacity(); shop.showAll(); break;
            case 7: shop.saveToFile("data.txt"); break;
            case 8: shop.loadFromFile("data.txt"); break;
        }
    } while (choice != 0);

    return 0;
}