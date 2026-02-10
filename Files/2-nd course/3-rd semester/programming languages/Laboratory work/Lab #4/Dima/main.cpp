#include "bank_data.h"

void showMenu() {
    std::cout << "\n===== МЕНЮ =====\n";
    std::cout << "1. Загрузить данные об ограблениях (из input.txt)\n";
    std::cout << "2. Выполнить анализ и показать ТОП-5 банков по кварталам\n";
    std::cout << "3. Сохранить отчет в файл (output.txt)\n";
    std::cout << "0. Закончить работу с программой\n";
    std::cout << "=========================================\n";
    std::cout << "Ваш выбор: ";
}

int main() {

    RobberyProcessor processor;
    int choice;

    do {
        showMenu();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                if (processor.loadFromFile("input.txt")) {
                    std::cout << "Данные успешно загружены из input.txt\n";
                } else {
                    std::cout << "Ошибка! Файл input.txt не найден.\n";
                    std::cout << "Создайте файл в формате: Месяц Банк Сумма %_Возмещения\n";
                }
                break;

            case 2:
                processor.processAndDisplay();
                break;

            case 3:
                if (processor.isEmpty()) {
                    std::cout << "Нет данных для сохранения.\n";
                } else if (processor.saveResultsToFile("output.txt")) {
                    std::cout << "Отчет сохранен в output.txt\n";
                } else {
                    std::cout << "Ошибка записи в файл!\n";
                }
                break;

            case 0:
                std::cout << "Программа завершена.\n";
                break;

            default:
                std::cout << "Неверный пункт меню.\n";
        }
    } while (choice != 0);

    return 0;
}