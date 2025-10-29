// robbery_analyzer.cpp
#include "robbery_analyzer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

// Реализация методов класса BankRobberyAnalyzer

void BankRobberyAnalyzer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Ошибка: Не удалось открыть файл для чтения: " + filename);
    }

    records.clear(); // Очищаем старые данные перед загрузкой новых
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        RobberyRecord record;
        
        // Используем std::ws для пропуска пробелов перед названием банка
        if (ss >> record.month >> std::ws && 
            std::getline(ss, record.bankName, ',') &&
            ss >> record.stolenAmount >> record.reimbursementPercent) 
        {
            // Убираем возможные пробелы в конце имени банка
            record.bankName.erase(record.bankName.find_last_not_of(" \n\r\t")+1);
            records.push_back(record);
        }
    }
    file.close();
}

void BankRobberyAnalyzer::addRecord(const RobberyRecord& record) {
    records.push_back(record);
}


void BankRobberyAnalyzer::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Ошибка: Не удалось открыть файл для записи: " + filename);
    }

    for (const auto& record : records) {
        file << record.month << " " << record.bankName << ", " 
             << record.stolenAmount << " " << record.reimbursementPercent << "\n";
    }
    file.close();
}


void BankRobberyAnalyzer::analyzeData() {
    topBanksByQuarter.clear();
    std::map<int, std::map<std::string, double>> quarterlyDamage;

    for (const auto& record : records) {
        int quarter = getQuarter(record.month);
        if (quarter < 1 || quarter > 4) continue; // Пропускаем неверные данные

        double actualDamage = record.stolenAmount * (1.0 - record.reimbursementPercent / 100.0);
        quarterlyDamage[quarter][record.bankName] += actualDamage;
    }

    for (auto const& [quarter, bankDamages] : quarterlyDamage) {
        std::vector<std::pair<std::string, double>> sortedBanks(bankDamages.begin(), bankDamages.end());

        // Сортировка банков по убыванию ущерба
        std::sort(sortedBanks.begin(), sortedBanks.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        // Оставляем только топ-5
        if (sortedBanks.size() > 5) {
            sortedBanks.resize(5);
        }

        topBanksByQuarter[quarter] = sortedBanks;
    }
}

std::string BankRobberyAnalyzer::getResultsAsString() const {
    if (topBanksByQuarter.empty()) {
        return "Данные не проанализированы или отсутствуют. Сначала выполните анализ.\n";
    }

    std::stringstream result;
    result << std::fixed << std::setprecision(2);

    for (int q = 1; q <= 4; ++q) {
        result << "--- Квартал " << q << " ---\n";
        if (topBanksByQuarter.count(q)) {
            const auto& topBanks = topBanksByQuarter.at(q);
            int rank = 1;
            for (const auto& bank : topBanks) {
                result << rank++ << ". " << bank.first << " | Ущерб: " << bank.second << "\n";
            }
        } else {
            result << "Нет данных по ограблениям за этот квартал.\n";
        }
        result << "\n";
    }
    return result.str();
}

int BankRobberyAnalyzer::getQuarter(int month) const {
    if (month >= 1 && month <= 3) return 1;
    if (month >= 4 && month <= 6) return 2;
    if (month >= 7 && month <= 9) return 3;
    if (month >= 10 && month <= 12) return 4;
    return 0; // Некорректный месяц
}