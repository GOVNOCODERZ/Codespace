#include <iostream>
#include <limits>
#include "storage_devices.h"

// Новая функция для добавления устройства вручную
void addDeviceManually(Shop& shop) {
    int typeChoice;
    std::cout << "\nКакой тип устройства добавить?\n";
    std::cout << "1. HDD (Жесткий диск)\n";
    std::cout << "2. FlashD (Флеш-накопитель)\n";
    std::cout << "Выберите тип: ";
    std::cin >> typeChoice;

    if (std::cin.fail() || (typeChoice != 1 && typeChoice != 2)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Неверный выбор. Возврат в главное меню.\n";
        return;
    }

    // Сбор общей информации
    std::string name, firm;
    int capacity;
    double price;

    std::cout << "Введите название модели: ";
    std::cin >> name;
    std::cout << "Введите фирму-производителя: ";
    std::cin >> firm;
    std::cout << "Введите объем (в GB): ";
    std::cin >> capacity;
    std::cout << "Введите цену: ";
    std::cin >> price;

    // Сбор специфичной информации и создание объекта
    if (typeChoice == 1) { // HDD
        int rpm;
        std::cout << "Введите скорость вращения (RPM): ";
        std::cin >> rpm;
        shop.addDevice(std::make_unique<HDD>(name, firm, capacity, price, rpm));
    } else { // FlashD
        std::string usb_type;
        std::cout << "Введите тип USB (например, 3.1): ";
        std::cin >> usb_type;
        shop.addDevice(std::make_unique<FlashD>(name, firm, capacity, price, usb_type));
    }

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Ошибка ввода данных. Устройство не добавлено.\n";
    } else {
        std::cout << "Устройство успешно добавлено!\n";
    }
}


void displayMenu() {
    std::cout << "\n--- Меню ---\n";
    std::cout << "1. Загрузить устройства из файла\n";
    std::cout << "2. Показать все устройства\n";
    std::cout << "3. Показать устройства в диапазоне объемов\n";
    std::cout << "4. Показать статистику (Объем/Цена)\n";
    std::cout << "5. Отсортировать устройства по объему\n";
    std::cout << "6. Добавить устройство вручную\n";
    std::cout << "7. Сохранить устройства в файл\n";
    std::cout << "0. Выход\n";
    std::cout << "Ваш выбор: ";
}

int main() {
    Shop myShop;
    
    myShop.addDevice(std::make_unique<HDD>("Barracuda", "Seagate", 2000, 5500.0, 7200));
    myShop.addDevice(std::make_unique<FlashD>("DataTraveler", "Kingston", 128, 1200.0, "3.1"));
    myShop.addDevice(std::make_unique<HDD>("WD Blue", "WD", 1000, 3800.0, 5400));
    myShop.addDevice(std::make_unique<FlashD>("Ultra Flair", "SanDisk", 256, 2100.0, "3.0"));

    int choice;
    do {
        displayMenu();
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1; 
        }

        switch (choice) {
            case 1: {
                std::string filename;
                std::cout << "Введите имя файла для загрузки (например, data.txt): ";
                std::cin >> filename;
                if (myShop.loadFromFile(filename)) {
                    std::cout << "Данные успешно загружены.\n";
                } else {
                    std::cout << "Ошибка: не удалось открыть файл.\n";
                }
                break;
            }
            case 2:
                std::cout << "\n--- Список всех устройств ---\n";
                myShop.printAll(std::cout);
                break;
            case 3: {
                int min_c, max_c;
                std::cout << "Введите минимальный объем (GB): ";
                std::cin >> min_c;
                std::cout << "Введите максимальный объем (GB): ";
                std::cin >> max_c;
                std::cout << std::endl;
                myShop.showByCapacityRange(std::cout, min_c, max_c);
                break;
            }
            case 4:
                std::cout << "\n--- Статистика ---\n";
                myShop.showCapacityPriceRatioStats(std::cout);
                break;
            case 5:
                myShop.sortByCapacity();
                std::cout << "Устройства отсортированы по объему.\n";
                break;
            case 6: // Новый case
                addDeviceManually(myShop);
                break;
            case 7: {
                 std::string filename;
                 std::cout << "Введите имя файла для сохранения (например, output.txt): ";
                 std::cin >> filename;
                 if (myShop.saveToFile(filename)) {
                     std::cout << "Данные успешно сохранены.\n";
                 } else {
                     std::cout << "Ошибка: не удалось сохранить в файл.\n";
                 }
                 break;
            }
            case 0:
                std::cout << "Завершение работы программы.\n";
                break;
            default:
                std::cout << "Неверный ввод. Пожалуйста, попробуйте снова.\n";
                break;
        }

    } while (choice != 0);

    return 0;
}