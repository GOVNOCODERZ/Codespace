#ifndef FIGURES_H
#define FIGURES_H

#include <iostream>
#include <cmath>
#include <algorithm>
#include "FigureExceptions.h"

struct Point {
    double x, y;
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "(" << p.x << ";" << p.y << ")";
        return os;
    }
};

class Square {
private:
    Point pos;
    double side;
public:
    Square(double s = 0) : side(s) {
        if (s < 0) throw ValidationError("Сторона не может быть отрицательной");
        pos.x = 0; pos.y = 0;
    }
    double getArea() const { return side * side; }

    bool operator>(const Square& other) const { return this->getArea() > other.getArea(); }
    bool operator<(const Square& other) const { return this->getArea() < other.getArea(); }

    friend std::ostream& operator<<(std::ostream& os, const Square& s) {
        if (&os == &std::cout) os << "Квадрат [S=" << s.getArea() << "]";
        else os << s.pos.x << " " << s.pos.y << " " << s.side;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Square& s) {
        double x, y, side;
        if (!(is >> x >> y >> side)) throw ValidationError("Неправильный формат ввода");
        if (side < 0) throw ValidationError("Сторона не может быть отрицательной");
        s.pos.x = x; s.pos.y = y; s.side = side;
        return is;
    }

    void setRandomValue() {
        side = (rand() % 100 + 1) / 10.0;
        pos.x = rand() % 10; pos.y = rand() % 10;
    }
};

class Circle {
private:
    Point center;
    double radius;
public:
    Circle(double r = 0) : radius(r) {
        if (r < 0) throw ValidationError("Радиус не может быть отрицательным");
        center.x = 0; center.y = 0;
    }
    double getArea() const { return 3.14159 * radius * radius; }

    bool operator>(const Circle& other) const { return this->getArea() > other.getArea(); }
    bool operator<(const Circle& other) const { return this->getArea() < other.getArea(); }

    friend std::ostream& operator<<(std::ostream& os, const Circle& c) {
        if (&os == &std::cout) os << "Круг [S=" << c.getArea() << "]";
        else os << c.center.x << " " << c.center.y << " " << c.radius;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Circle& c) {
        double x, y, r;
        if (!(is >> x >> y >> r)) throw ValidationError("Неправильный формат ввода");
        if (r < 0) throw ValidationError("Радиус не может быть отрицательным");
        c.center.x = x; c.center.y = y; c.radius = r;
        return is;
    }

    void setRandomValue() {
        radius = (rand() % 100 + 1) / 10.0;
        center.x = rand() % 10; center.y = rand() % 10;
    }
};
#endif