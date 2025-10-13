#include "payment.h"
#include <fstream>
#include <algorithm>
#include <iomanip>

// Конструкторы

Payment::Payment() : date(""), amount(0.0f) {}
Payment::Payment(const string& date, float amount) : date(date), amount(amount) {}
Payment::Payment(const Payment& other) : date(other.date), amount(other.amount) {}
Payment::~Payment() {}

/// @brief Заполнение полей объекта
void Payment::input(istream& is) {
    if (&is == &cin) {
        cout << "Enter date: ";
    }
    is >> date;

    if (&is == &cin) {
        cout << "Enter amount: ";
    }
    is >> amount;
}

/// @brief Вывод полей объекта
void Payment::output(ostream& os) const {
    os << "Date: " << date << ", Amount: " << amount;
}

/// @brief Надстройка над input()
/// @param is Входящий поток
/// @param p Объект для ввода
/// @return Изменённый поток
istream& operator>>(istream& is, Payment& p) {
    p.input(is);
    return is;
}

/// @brief Надстройка над output()
/// @param os Исходящий поток
/// @param p Объект для вывода
/// @return Изменённый поток
ostream& operator<<(ostream& os, const Payment& p) {
    p.output(os);
    return os;
}



// Конструкторы

BankTransfer::BankTransfer() : Payment(), bank("") {}
BankTransfer::BankTransfer(const string& date, float amount, const string& bank)
    : Payment(date, amount), bank(bank) {}
BankTransfer::BankTransfer(const BankTransfer& other)
    : Payment(other), bank(other.bank) {}
BankTransfer::~BankTransfer() {}

/// @brief Геттер для названия объекта
/// @return Строка с названием
string BankTransfer::myName() const {
    return "BankTransfer";
}

/// @brief Заполнение полей объекта
void BankTransfer::input(istream& is) {
    Payment::input(is);
    if (&is == &cin) {
        cout << "Enter bank: ";
    }
    is >> bank;
}

/// @brief Вывод полей объекта
void BankTransfer::output(ostream& os) const {
    Payment::output(os);
    os << ", Bank: " << bank;
}

/// @brief Надстройка над input()
/// @param is Входящий поток
/// @param p Объект для ввода
/// @return Изменённый поток
istream& operator>>(istream& is, BankTransfer& b) {
    b.input(is);
    return is;
}

/// @brief Надстройка над output()
/// @param os Исходящий поток
/// @param p Объект для вывода
/// @return Изменённый поток
ostream& operator<<(ostream& os, const BankTransfer& b) {
    b.output(os);
    return os;
}


// Конструкторы

WebMoney::WebMoney() : Payment(), commission(0.0f) {}
WebMoney::WebMoney(const string& date, float amount, float commission)
    : Payment(date, amount), commission(commission) {}
WebMoney::WebMoney(const WebMoney& other)
    : Payment(other), commission(other.commission) {}
WebMoney::~WebMoney() {}

/// @brief Геттер для названия объекта
/// @return Строка с названием
string WebMoney::myName() const {
    return "WebMoney";
}

/// @brief Заполнение полей объекта
void WebMoney::input(istream& is) {
    Payment::input(is);
    if (&is == &cin) {
        cout << "Enter commission: ";
    }
    is >> commission;
}

/// @brief Вывод полей объекта
void WebMoney::output(ostream& os) const {
    Payment::output(os);
    os << ", Commission: " << commission;
}

/// @brief Надстройка над input()
/// @param is Входящий поток
/// @param p Объект для ввода
/// @return Изменённый поток
istream& operator>>(istream& is, WebMoney& w) {
    w.input(is);
    return is;
}

/// @brief Надстройка над output()
/// @param os Исходящий поток
/// @param p Объект для вывода
/// @return Изменённый поток
ostream& operator<<(ostream& os, const WebMoney& w) {
    w.output(os);
    return os;
}


// Конструкторы

Sales::Sales() {}
Sales::Sales(const Sales& other) {
    for (auto p : other.payments) {
        Payment* obj;
        obj = p;
        payments.push_back(obj);
    }
}
Sales::~Sales() {
    for (auto p : payments) {
        delete p;
    }
}

/// @brief Добавление объекта в массив
/// @param p Поинтер на массив
void Sales::addPayment(Payment* p) {
    payments.push_back(p);
}

/// @brief Вывод всех объектов массива
void Sales::displayAll() const {
    for (auto p : payments) {
        cout << p->myName() << ": ";
        cout << p << endl;
    }
}

/// @brief Вывод объектов из массива на заданную дату
/// @param date Заданная дата
void Sales::selectByDate(const string& date) const {
    cout << "\n=== Payments on " << date << " ===" << endl;
    bool found = false;
    for (auto p : payments) {
        if (p->getDate() == date) {
            cout << p->myName() << ": ";
            cout << p << endl;
            found = true;
        }
    }
    if (!found) {
        cout << "No payments found on this date." << endl;
    }
}

/// @brief Вывод объектов из массива в диапазоне величины платежа (включительно)
/// @param min Минимальная допустимая величина
/// @param max Максимальная допустимая величина
void Sales::selectByAmountRange(float min, float max) const {
    cout << "\n=== Payments between " << min << " and " << max << " ===" << endl;
    bool found = false;
    for (auto p : payments) {
        if (p->getAmount() >= min && p->getAmount() <= max) {
            cout << p->myName() << ": ";
            cout << p << endl;
            found = true;
        }
    }
    if (!found) {
        cout << "No payments found in this range." << endl;
    }
}

/// @brief Статистическая сводка по всем типам платежа. 
/// Количество операцией, общая сумма по типу, процент от суммы операций всех типов
void Sales::showStatsByType() const {
    int bankCount = 0, webCount = 0;
    float bankSum = 0, webSum = 0;

    for (auto p : payments) {
        if (p->myName() == "BankTransfer") {
            bankCount++;
            bankSum += p->getAmount();
        } else if (p->myName() == "WebMoney") {
            webCount++;
            webSum += p->getAmount();
        }
    }

    float total = bankSum + webSum;

    cout << "\n=== Statistics by Type ===" << endl;
    cout << "BankTransfer: " << bankCount << " payments, sum = " << bankSum
         << ", share = " << fixed << setprecision(2) << (total ? (bankSum / total) * 100 : 0) << "%" << endl;
    cout << "WebMoney: " << webCount << " payments, sum = " << webSum
         << ", share = " << fixed << setprecision(2) << (total ? (webSum / total) * 100 : 0) << "%" << endl;
}

/// @brief Загрузка данных из файла
/// @param filename Файл с данными
void Sales::loadFromFile(const string& filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }

    payments.clear();
    
    string type;
    while (file >> type) {
        Payment* obj;
        if (type == "BankTransfer") {
            obj = new BankTransfer;
            file >> *obj;
            addPayment(obj);
        }
        if (type == "WebMoney") {
            obj = new WebMoney;
            file >> *obj;
            addPayment(obj);
        }
    }
    
    file.close();
    cout << "Data loaded from " << filename << endl;
}

/// @brief Запись данных в файл
/// @param filename Файл для записи
void Sales::saveToFile(const string& filename) const {
    ofstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }

    for (auto p : payments) {
        file << p->myName() << " " << p->getDate() << " " << p->getAmount() << " ";
        if (auto* bt = dynamic_cast<BankTransfer*>(p)) {
            file << bt->getBank();
        }
        if (auto* wm = dynamic_cast<WebMoney*>(p)) {
            file << wm->getCommission();
        }
        file << endl;
    }

    file.close();
    cout << "Data saved to " << filename << endl;
}