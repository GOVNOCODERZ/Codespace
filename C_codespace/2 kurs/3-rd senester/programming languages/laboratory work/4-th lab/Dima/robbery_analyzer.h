// robbery_analyzer.h
#ifndef ROBBERY_ANALYZER_H
#define ROBBERY_ANALYZER_H

#include <string>
#include <vector>
#include <map>

// Структура для хранения информации об одном ограблении
struct RobberyRecord {
    int month;
    std::string bankName;
    double stolenAmount;
    double reimbursementPercent;
};

// Класс-анализатор данных об ограблениях банков
class BankRobberyAnalyzer {
public:
    // Конструктор
    BankRobberyAnalyzer() = default;

    // Загрузка данных из файла
    void loadFromFile(const std::string& filename);

    // Добавление новой записи (для интерактивного режима)
    void addRecord(const RobberyRecord& record);
    
    // Сохранение текущих данных в файл (перезаписывает файл)
    void saveToFile(const std::string& filename);

    // Основной метод для анализа данных
    void analyzeData();

    // Получение результатов анализа в виде отформатированной строки
    std::string getResultsAsString() const;

private:
    // Вектор для хранения всех записей об ограблениях
    std::vector<RobberyRecord> records;

    // Результаты анализа: для каждого квартала (ключ int) храним отсортированный список
    // пар (название банка, ущерб)
    std::map<int, std::vector<std::pair<std::string, double>>> topBanksByQuarter;

    // Вспомогательная функция для определения квартала по месяцу
    int getQuarter(int month) const;
};

#endif // ROBBERY_ANALYZER_H