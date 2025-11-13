#ifndef FRACTION_H
#define FRACTION_H

#include <iostream>
#include <cmath>
using namespace std;

/**
 * @brief Дробное число, состоящее из числителя и знаменателя.
 */
class Fraction {
private:
    // Числитель
    int num;
    // Знаменатель
    int den;

    /**
     * @brief Вспомогательная функция для вычисления наибольшего общего делителя (НОД).
     * Используется для сокращения дроби.
     * 
     * @param a Первое число.
     * @param b Второе число.
     * @return Наибольший общий делитель чисел a и b.
     */
    int gcd(int a, int b) const {
        a = abs(a); // Берем абсолютное значение, чтобы НОД был положительным
        b = abs(b);
        while (b) { // Алгоритм Евклида
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    /**
     * @brief Вспомогательная функция для нормализации дроби.
     * Приводит дробь к каноническому виду: знаменатель положительный, дробь сокращена.
     */
    void normalize() {
        if (den < 0) { // Если знаменатель отрицательный, переносим знак на числитель
            num = -num;
            den = -den;
        }
        int g = gcd(num, den); // Вычисляем НОД
        if (g != 0) { // Если НОД не равен 0, сокращаем дробь
            num /= g;
            den /= g;
        }
    }

public:
    /**
     * @brief Конструктор из двух целых чисел.
     * Создаёт дробь num/den. Выполняет нормализацию.
     * 
     * @param n Числитель. По умолчанию 0.
     * @param d Знаменатель. По умолчанию 1.
     */
    Fraction(int n = 0, int d = 1) : num(n), den(d) {
        if (den == 0) { // Защита от деления на ноль
            cout << "ERROR: Denominator cannot be zero!" << endl;
            den = 1; // Устанавливаем в 1, чтобы избежать сбоя
        }
        normalize(); // Приводим к каноническому виду
    }

    /**
     * @brief Конструктор из double.
     * Преобразует вещественное число в дробь с ограниченной точностью.
     * Например, 0.1 -> 1/10.
     * 
     * @param d Вещественное число для преобразования.
     */
    Fraction(double d) {
        const int precision = 100; // Максимальный знаменатель ~100 (2 знака после запятой)
        double scaled_val = d * precision; // Масштабируем до целого
        int n = static_cast<int>(round(scaled_val)); // Округляем до ближайшего целого
        int g = gcd(n, precision); // НОД для сокращения
        num = n / g;
        den = precision / g;
        normalize(); // Финальная нормализация
    }

    // Возвращает числитель
    int getNum() const { return num; } 
    // Возвращает знаменатель
    int getDen() const { return den; } 

    /**
     * @brief Оператор "меньше".
     * Сравнивает две дроби, приводя к общему знаменателю.
     * 
     * @param other Вторая дробь для сравнения.
     * @return true, если текущая дробь меньше, иначе false.
     */
    bool operator<(const Fraction& other) const {
        return (num * other.den) < (other.num * den); // Приведение к общему знаменателю перед сравнением
    }

    /**
     * @brief Оператор "больше".
     * 
     * @param other Вторая дробь для сравнения.
     * @return true, если текущая дробь больше, иначе false.
     */
    bool operator>(const Fraction& other) const {
        return (num * other.den) > (other.num * den);
    }

    /**
     * @brief Оператор "равно".
     * 
     * @param other Вторая дробь для сравнения.
     * @return true, если дроби равны (с учётом сокращения), иначе false.
     */
    bool operator==(const Fraction& other) const {
        return (num == other.num && den == other.den);
    }

    /**
     * @brief Оператор "не равно".
     * 
     * @param other Вторая дробь для сравнения.
     * @return true, если дроби не равны, иначе false.
     */
    bool operator!=(const Fraction& other) const {
        return !(*this == other); // Использует operator==
    }

    /**
     * @brief Оператор "меньше или равно".
     * 
     * @param other Вторая дробь для сравнения.
     * @return true, если текущая дробь <= other, иначе false.
     */
    bool operator<=(const Fraction& other) const {
        return (*this < other) || (*this == other);
    }

    /**
     * @brief Оператор "больше или равно".
     * 
     * @param other Вторая дробь для сравнения.
     * @return true, если текущая дробь >= other, иначе false.
     */
    bool operator>=(const Fraction& other) const {
        return (*this > other) || (*this == other);
    }

    // Операторы арифметики
    /**
     * @brief Оператор сложения.
     * Складывает две дроби: a/b + c/d = (a*d + c*b) / (b*d).
     * 
     * @param other Вторая дробь.
     * @return Новую дробь, равную сумме.
     */
    Fraction operator+(const Fraction& other) const {
        return Fraction(num * other.den + other.num * den, den * other.den);
    }

    /**
     * @brief Оператор вычитания.
     * Вычитает: a/b - c/d = (a*d - c*b) / (b*d).
     * 
     * @param other Вторая дробь.
     * @return Новую дробь, равную разности.
     */
    Fraction operator-(const Fraction& other) const {
        return Fraction(num * other.den - other.num * den, den * other.den);
    }

    /**
     * @brief Оператор умножения.
     * Умножает: a/b * c/d = (a*c) / (b*d).
     * 
     * @param other Вторая дробь.
     * @return Новую дробь, равную произведению.
     */
    Fraction operator*(const Fraction& other) const {
        return Fraction(num * other.num, den * other.den);
    }

    /**
     * @brief Оператор деления.
     * Делит: a/b / c/d = (a*d) / (b*c).
     * Проверяет деление на ноль.
     * 
     * @param other Дробь-делитель.
     * @return Новую дробь, равную частному.
     */
    Fraction operator/(const Fraction& other) const {
        if (other.num == 0) { // Проверка деления на ноль
            cout << "ERROR: Division by zero!" << endl;
            return Fraction(0, 1); // Возвращаем ноль
        }
        return Fraction(num * other.den, den * other.num);
    }

    // Проверяет, положительна ли дробь
    bool isPositive() const { return num > 0; } 
    // Проверяет, отрицательна ли дробь
    bool isNegative() const { return num < 0; } 

    /**
     * @brief Преобразование дроби в double.
     * Может использоваться для приближённых вычислений или генерации случайных значений.
     * 
     * @return Значение дроби как double.
     */
    double toDouble() const {
        return static_cast<double>(num) / den;
    }

    /**
     * @brief Оператор ввода из потока.
     * Читает дробь в формате "num/den" или "num" (если знаменатель 1).
     * 
     * @param is Входной поток (например, cin или ifstream).
     * @param f Объект дроби для заполнения.
     * @return Ссылку на входной поток.
     */
    friend istream& operator>>(istream& is, Fraction& f) {
        string s;
        is >> s; // Читаем всю строку до пробела

        size_t pos = s.find('/'); // Ищем символ '/'
        if (pos == string::npos) { // Если '/' нет — это целое число
            f.num = stoi(s);
            f.den = 1;
        } else { // Если '/' есть — разбиваем строку
            string num_str = s.substr(0, pos); // До '/'
            string den_str = s.substr(pos + 1); // После '/'
            f.num = stoi(num_str);
            f.den = stoi(den_str);
            if (f.den == 0) { // Проверка на деление на ноль
                cout << "ERROR: Denominator cannot be zero! Setting to 1." << endl;
                f.den = 1;
            }
        }
        f.normalize(); // Нормализуем дробь после ввода
        return is;
    }

    /**
     * @brief Оператор вывода в поток.
     * Выводит дробь в формате "num/den", если знаменатель != 1, иначе только "num".
     * 
     * @param os Выходной поток (например, cout или ofstream).
     * @param f Объект дроби для вывода.
     * @return Ссылку на выходной поток.
     */
    friend ostream& operator<<(ostream& os, const Fraction& f) {
        os << f.num; // Выводим числитель
        if (f.den != 1) os << "/" << f.den; // Если знаменатель != 1, выводим "/"
        return os;
    }
};

#endif