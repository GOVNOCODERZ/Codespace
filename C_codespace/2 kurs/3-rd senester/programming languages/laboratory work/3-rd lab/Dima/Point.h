#ifndef POINT_H
#define POINT_H

#include <iostream>

// Простая структура для представления 2D точки
struct Point {
    double x = 0.0;
    double y = 0.0;
};

// Перегрузка оператора ввода для Point
// Добавлено ключевое слово 'inline'
inline std::istream& operator>>(std::istream& is, Point& p) {
    // Ожидаемый формат: x y
    is >> p.x >> p.y;
    return is;
}

// Перегрузка оператора вывода для Point
inline std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

#endif // POINT_H