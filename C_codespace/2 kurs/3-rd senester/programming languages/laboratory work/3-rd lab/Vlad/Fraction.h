#ifndef FRACTION_H
#define FRACTION_H

#include <iostream>
#include <cmath> // для abs
using namespace std;

class Fraction {
private:
    int num; // числитель
    int den; // знаменатель

    // Вспомогательная функция для сокращения дроби
    int gcd(int a, int b) const {
        a = abs(a);
        b = abs(b);
        while (b) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    void normalize() {
        if (den < 0) {
            num = -num;
            den = -den;
        }
        int g = gcd(num, den);
        if (g != 0) {
            num /= g;
            den /= g;
        }
    }

public:
    // Конструктор из int (по умолчанию)
    Fraction(int n = 0, int d = 1) : num(n), den(d) {
        if (den == 0) {
            cout << "Error: Denominator cannot be zero!" << endl;
            den = 1;
        }
        normalize();
    }

    // --- НОВЫЙ КОНСТРУКТОР ИЗ ЧИСЛИТЕЛЯ И ЗНАМЕНАТЕЛЯ ---
    Fraction(int n, int d, bool dummy) : num(n), den(d) {
        // Этот конструктор НЕ вызывает normalize, потому что предполагается, что дробь уже сокращена
        // dummy нужен только для отличия от обычного конструктора
        if (den == 0) {
            cout << "Error: Denominator cannot be zero!" << endl;
            den = 1;
        }
        normalize(); // Всё равно нормализуем, чтобы избежать проблем
    }
    // --- КОНЕЦ НОВОГО КОНСТРУКТОРА ---

    // --- КОНСТРУКТОР ИЗ DOUBLE С ОКРУГЛЕНИЕМ ---
    // Пример: 0.1 -> 1/10
    Fraction(double d) {
        const int precision = 100; // Максимальный знаменатель ~100
        double scaled_val = d * precision;
        int n = static_cast<int>(round(scaled_val));
        int g = gcd(n, precision);
        num = n / g;
        den = precision / g;
        normalize();
    }
    // --- КОНЕЦ КОНСТРУКТОРА ---

    // Геттеры
    int getNum() const { return num; }
    int getDen() const { return den; }

    // Операторы для сравнения и арифметики, необходимые для шаблона
    bool operator<(const Fraction& other) const {
        return (num * other.den) < (other.num * den);
    }

    bool operator>(const Fraction& other) const {
        return (num * other.den) > (other.num * den);
    }

    bool operator==(const Fraction& other) const {
        return (num == other.num && den == other.den);
    }

    bool operator!=(const Fraction& other) const {
        return !(*this == other);
    }

    bool operator<=(const Fraction& other) const {
        return (*this < other) || (*this == other);
    }

    bool operator>=(const Fraction& other) const {
        return (*this > other) || (*this == other);
    }

    Fraction operator+(const Fraction& other) const {
        return Fraction(num * other.den + other.num * den, den * other.den);
    }

    Fraction operator-(const Fraction& other) const {
        return Fraction(num * other.den - other.num * den, den * other.den);
    }

    Fraction operator*(const Fraction& other) const {
        return Fraction(num * other.num, den * other.den);
    }

    Fraction operator/(const Fraction& other) const {
        if (other.num == 0) {
            cout << "Error: Division by zero!" << endl;
            return Fraction(0, 1);
        }
        return Fraction(num * other.den, den * other.num);
    }

    // Операторы для работы с отрицательными значениями
    bool isPositive() const {
        return num > 0;
    }

    bool isNegative() const {
        return num < 0;
    }

    // Преобразование в double для арифметики (необязательно, но может помочь)
    double toDouble() const {
        return static_cast<double>(num) / den;
    }

    // Ввод/вывод
    friend istream& operator>>(istream& is, Fraction& f) {
        string s;
        is >> s; // Читаем всю строку до пробела

        size_t pos = s.find('/');
        if (pos == string::npos) {
            // Если '/' нет — значит, целое число
            f.num = stoi(s);
            f.den = 1;
        } else {
            // Разбиваем строку по '/'
            string num_str = s.substr(0, pos);
            string den_str = s.substr(pos + 1);
            f.num = stoi(num_str);
            f.den = stoi(den_str);
            if (f.den == 0) {
                cout << "Error: Denominator cannot be zero! Setting to 1." << endl;
                f.den = 1;
            }
        }
        f.normalize();
        return is;
    }

    friend ostream& operator<<(ostream& os, const Fraction& f) {
        os << f.num; // Выводим числитель
        if (f.den != 1) os << "/" << f.den; // Если знаменатель != 1, выводим "/"
        return os;
    }
};

#endif