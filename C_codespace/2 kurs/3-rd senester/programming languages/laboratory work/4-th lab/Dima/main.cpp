// main.cpp
#include <iostream>
#include <string>
#include <limits>
#include "robbery_analyzer.h"

void displayMenu() {
    std::cout << "\n===== Меню =====\n";
    std::cout << "1. Загрузить данные из файла\n";
    std::cout << "2. Добавить новую запись об ограблении\n";
    std::cout << "3. Проанализировать данные\n";
    std::cout << "4. Показать результаты анализа\n";
    std::cout << "5. Сохранить данные в файл\n";
    std::cout << "0. Выход\n";
    std::cout << "Выберите опцию: ";
}

void addNewRecord(BankRobberyAnalyzer& analyzer) {
    RobberyRecord newRecord;
    
    std::cout << "Введите месяц (1-12): ";
    while (!(std::cin >> newRecord.month) || newRecord.month < 1 || newRecord.month > 12) {
        std::cout << "Некорректный ввод. Введите число от 1 до 12: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Введите название банка: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Очистка буфера перед getline
    std::getline(std::cin, newRecord.bankName);

    std::cout << "Введите сумму украденного: ";
    while (!(std::cin >> newRecord.stolenAmount) || newRecord.stolenAmount < 0) {
        std::cout << "Некорректный ввод. Введите положительное число: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Введите процент возмещения (0-100): ";
    while (!(std::cin >> newRecord.reimbursementPercent) || newRecord.reimbursementPercent < 0 || newRecord.reimbursementPercent > 100) {
        std::cout << "Некорректный ввод. Введите число от 0 до 100: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    analyzer.addRecord(newRecord);
    std::cout << "Запись успешно добавлена. Не забудьте сохранить изменения в файл.\n";
}


int main() {
    BankRobberyAnalyzer analyzer;
    std::string filename = "data.txt";
    int choice;

    do {
        displayMenu();
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1; // Некорректный выбор
        }

        switch (choice) {
            case 1:
                std::cout << "Введите имя файла для загрузки (например, data.txt): ";
                std::cin >> filename;
                try {
                    analyzer.loadFromFile(filename);
                    std::cout << "Данные успешно загружены из файла " << filename << std::endl;
                } catch (const std::runtime_error& e) {
                    std::cerr << e.what() << std::endl;
                }
                break;
            case 2:
                addNewRecord(analyzer);
                break;
            case 3:
                analyzer.analyzeData();
                std::cout << "Анализ данных завершен.\n";
                break;
            case 4:
                std::cout << "\n--- Результаты анализа ---\n";
                std::cout << analyzer.getResultsAsString();
                break;
            case 5:
                 std::cout << "Введите имя файла для сохранения (например, data.txt): ";
                 std::cin >> filename;
                 try {
                    analyzer.saveToFile(filename);
                    std::cout << "Данные успешно сохранены в файл " << filename << std::endl;
                } catch (const std::runtime_error& e) {
                    std::cerr << e.what() << std::endl;
                }
                break;
            case 0:
                std::cout << "Выход из программы.\n";
                break;
            default:
                std::cout << "Неверный выбор. Пожалуйста, попробуйте снова.\n";
                break;
        }
    } while (choice != 0);

    return 0;
}