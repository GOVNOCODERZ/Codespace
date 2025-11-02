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

    // --- НОВЫЙ КОНСТРУКТОР ИЗ DOUBLE ---
    // Пример: 3.14 -> 314/100
    Fraction(double d) {
        const long long scale = 1000000LL; // Масштабируем до 6 знаков после запятой
        long long scaled_val = static_cast<long long>(d * scale);
        long long g = gcd(scaled_val, scale);
        num = scaled_val / g;
        den = scale / g;
        normalize();
    }
    // --- КОНЕЦ НОВОГО КОНСТРУКТОРА ---

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
        cout << "Enter numerator: ";
        is >> f.num;
        cout << "Enter denominator: ";
        is >> f.den;
        if (f.den == 0) {
            cout << "Error: Denominator cannot be zero! Setting to 1." << endl;
            f.den = 1;
        }
        f.normalize();
        return is;
    }

    friend ostream& operator<<(ostream& os, const Fraction& f) {
        os << f.num;
        if (f.den != 1) os << "/" << f.den;
        return os;
    }
};

#endif