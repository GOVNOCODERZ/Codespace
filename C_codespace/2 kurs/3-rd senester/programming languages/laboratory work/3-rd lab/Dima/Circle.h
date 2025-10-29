#ifndef CIRCLE_H
#define CIRCLE_H

#include <iostream>
#include <cmath>
#include <random>
#include "Point.h"

class Circle {
private:
    Point center;
    double radius;
    // Константа PI для вычисления площади
    static constexpr double PI = 3.14159265358979323846;

public:
    // Конструктор по умолчанию
    Circle() : center{0, 0}, radius(0.0) {}

    // Параметризованный конструктор
    Circle(Point c, double r) : center(c), radius(r > 0 ? r : 0.0) {}

    // Метод для получения площади
    double getArea() const {
        return PI * radius * radius;
    }

    // Метод для генерации случайных значений для объекта
    void randomize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis_coord(-100.0, 100.0);
        std::uniform_real_distribution<> dis_rad(1.0, 50.0);

        center.x = dis_coord(gen);
        center.y = dis_coord(gen);
        radius = dis_rad(gen);
    }

    // Операторы сравнения, основанные на площади
    bool operator>(const Circle& other) const { return this->getArea() > other.getArea(); }
    bool operator<(const Circle& other) const { return this->getArea() < other.getArea(); }
    bool operator==(const Circle& other) const { return std::abs(this->getArea() - other.getArea()) < 1e-9; }

    // Дружественные функции для перегрузки операторов потокового ввода/вывода
    friend std::istream& operator>>(std::istream& is, Circle& c);
    friend std::ostream& operator<<(std::ostream& os, const Circle& c);
};

// Реализация оператора ввода
inline std::istream& operator>>(std::istream& is, Circle& c) {
    // Ожидаемый формат: center.x center.y radius
    is >> c.center.x >> c.center.y >> c.radius;
    return is;
}

// Реализация оператора вывода
inline std::ostream& operator<<(std::ostream& os, const Circle& c) {
    os << "Круг - Центр: " << c.center << ", Радиус: " << c.radius << ", Площадь: " << c.getArea();
    return os;
}

#endif // CIRCLE_H