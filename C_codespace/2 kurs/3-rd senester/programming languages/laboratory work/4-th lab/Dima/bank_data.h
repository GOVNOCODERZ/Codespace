#ifndef BANK_DATA_H
#define BANK_DATA_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <iomanip>

// --- Псевдонимы типов для удобства (чтобы не писать длинные конструкции) ---
using BankStat = std::pair<std::string, double>; // Имя банка + Ущерб
using QuarterStats = std::vector<BankStat>;      // Список банков за квартал
using ReportData = std::map<int, QuarterStats>;  // Квартал -> Список банков

// Структура для хранения записи об ограблении
struct RobberyRecord {
    int month;
    std::string bankName;
    double stolenAmount;
    double reimbursementPercent;

    // Расчет чистого ущерба: Сумма * (1 - Процент/100)
    double getActualDamage() const {
        return stolenAmount * (1.0 - reimbursementPercent / 100.0);
    }

    // Определение квартала по месяцу
    int getQuarter() const {
        if (month >= 1 && month <= 3) return 1;
        if (month >= 4 && month <= 6) return 2;
        if (month >= 7 && month <= 9) return 3;
        if (month >= 10 && month <= 12) return 4;
        return 0;
    }
};

class RobberyProcessor {
private:
    std::vector<RobberyRecord> records;

    // Вспомогательный метод: собирает, суммирует, сортирует и обрезает данные
    ReportData getSortedStatistics(size_t topCount) const;

public:
    // Загрузка данных из файла
    bool loadFromFile(const std::string& filename);
    
    // Сохранение результатов в файл
    bool saveResultsToFile(const std::string& filename);

    // Основная обработка и вывод на экран
    void processAndDisplay();

    // Очистка данных
    void clear() { records.clear(); }
    
    bool isEmpty() const { return records.empty(); }
};

#endif