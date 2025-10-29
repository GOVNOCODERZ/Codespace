#ifndef SQUARE_H
#define SQUARE_H

#include <iostream>
#include <cmath>
#include <random>
#include "Point.h"

class Square {
private:
    Point topLeft;
    double side;

public:
    // Конструктор по умолчанию
    Square() : topLeft{0, 0}, side(0.0) {}

    // Параметризованный конструктор
    Square(Point tl, double s) : topLeft(tl), side(s > 0 ? s : 0.0) {}

    // Метод для получения площади
    double getArea() const {
        return side * side;
    }
    
    // Метод для генерации случайных значений для объекта
    void randomize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis_coord(-100.0, 100.0);
        std::uniform_real_distribution<> dis_side(1.0, 50.0);

        topLeft.x = dis_coord(gen);
        topLeft.y = dis_coord(gen);
        side = dis_side(gen);
    }

    // Операторы сравнения, основанные на площади
    bool operator>(const Square& other) const { return this->getArea() > other.getArea(); }
    bool operator<(const Square& other) const { return this->getArea() < other.getArea(); }
    bool operator==(const Square& other) const { return std::abs(this->getArea() - other.getArea()) < 1e-9; }

    // Дружественные функции для перегрузки операторов потокового ввода/вывода
    friend std::istream& operator>>(std::istream& is, Square& s);
    friend std::ostream& operator<<(std::ostream& os, const Square& s);
};

// Реализация оператора ввода
inline std::istream& operator>>(std::istream& is, Square& s) {
    // Ожидаемый формат: topLeft.x topLeft.y side
    is >> s.topLeft.x >> s.topLeft.y >> s.side;
    return is;
}

// Реализация оператора вывода
inline std::ostream& operator<<(std::ostream& os, const Square& s) {
    os << "Квадрат - Левый верх: " << s.topLeft << ", Сторона: " << s.side << ", Площадь: " << s.getArea();
    return os;
}

#endif // SQUARE_H