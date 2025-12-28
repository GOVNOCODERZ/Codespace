#include "bank_data.h"
#include <sstream>

// Реализация приватного вспомогательного метода
ReportData RobberyProcessor::getSortedStatistics(size_t topCount) const {
    ReportData result;
    
    // 1. Агрегация: Квартал -> (Банк -> Суммарный ущерб)
    std::map<int, std::map<std::string, double>> tempMap;
    for (const auto& rec : records) {
        tempMap[rec.getQuarter()][rec.bankName] += rec.getActualDamage();
    }

    // 2. Преобразование в вектор, сортировка и обрезка
    for (const auto& [quarter, banksMap] : tempMap) {
        // Копируем из map в vector
        QuarterStats sortedBanks(banksMap.begin(), banksMap.end());

        // Сортируем по убыванию ущерба
        std::sort(sortedBanks.begin(), sortedBanks.end(), 
            [](const BankStat& a, const BankStat& b) {
                return a.second > b.second;
            });

        // Оставляем только топ-N (если их больше, чем нужно)
        if (sortedBanks.size() > topCount) {
            sortedBanks.resize(topCount);
        }
        
        result[quarter] = sortedBanks;
    }
    
    return result;
}

bool RobberyProcessor::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    records.clear();
    RobberyRecord rec;
    
    while (file >> rec.month >> rec.bankName >> rec.stolenAmount >> rec.reimbursementPercent) {
        records.push_back(rec);
    }

    file.close();
    return true;
}

void RobberyProcessor::processAndDisplay() {
    if (records.empty()) {
        std::cout << "Данные отсутствуют. Сначала загрузите файл.\n";
        return;
    }

    // Получаем готовые данные (ТОП-5)
    ReportData data = getSortedStatistics(5);

    std::cout << "\n=== АНАЛИЗ УЩЕРБА ПО КВАРТАЛАМ ===\n";
    
    for (int q = 1; q <= 4; ++q) {
        std::cout << "\nКвартал №" << q << ":\n";
        
        // Проверяем, есть ли данные по этому кварталу
        if (data.find(q) == data.end() || data[q].empty()) {
            std::cout << "  Нет данных за этот квартал.\n";
            continue;
        }

        const auto& banks = data[q];
        int count = 0;
        for (const auto& bank : banks) {
            std::cout << std::fixed << std::setprecision(2)
                      << "  " << ++count << ". " << std::left << std::setw(15) 
                      << bank.first << " | Ущерб: " << bank.second << "\n";
        }
    }
}

bool RobberyProcessor::saveResultsToFile(const std::string& filename) {
    if (records.empty()) return false;

    std::ofstream fout(filename);
    if (!fout.is_open()) return false;

    // Получаем готовые данные (ТОП-5)
    ReportData data = getSortedStatistics(5);

    for (const auto& [q, banks] : data) {
        fout << "Квартал " << q << ":\n";
        
        for (const auto& bank : banks) {
            fout << bank.first << " " << bank.second << "\n";
        }
        fout << "\n";
    }
    fout.close();
    return true;
}