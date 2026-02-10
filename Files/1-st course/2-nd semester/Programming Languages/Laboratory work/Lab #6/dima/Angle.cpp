#include "Angle.h"

void Angle::input() {
    std::cout << "Введите наименование угла: ";
    std::cin >> name;
    std::cout << "Введите градусы: ";
    std::cin >> degrees;
    std::cout << "Введите минуты: ";
    std::cin >> minutes;
    std::cout << "Введите секунды: ";
    std::cin >> seconds;
}

void Angle::display() const {
    std::cout << name << ": " << degrees << "° " << minutes << "' " << seconds << "\"\n";
}

int Angle::toSeconds() const {
    return degrees * 3600 + minutes * 60 + seconds;
}