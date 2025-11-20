#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iomanip>

using namespace std;

// Базовый абстрактный класс (Накопитель)
class StorageDev {
protected:
    // Поля protected для прямого доступа в наследниках
    string name;          // Название
    string manufacturer;  // Фирма
    double price;         // Цена
    int capacity;         // Емкость

public:
    // Конструкторы
    StorageDev();
    StorageDev(string n, string m, double p, int c);
    StorageDev(const StorageDev& other); // Конструктор копирования
    // Виртуальный деструктор
    virtual ~StorageDev();
    // Чистый виртуальный метод
    virtual string myName() const = 0;
    // Виртуальные методы ввода-вывода
    virtual void input(istream& is);
    virtual void print(ostream& os) const;
    // Геттеры
    int getCapacity() const { return capacity; }
    double getPrice() const { return price; }
    string getName() const { return name; }
    string getManufacturer() const { return manufacturer; }
    // Перегрузка операторов
    friend ostream& operator<<(ostream& os, const StorageDev& obj);
    friend istream& operator>>(istream& is, StorageDev& obj);
};

 
// Класс HDD (Жесткий диск)
class HDD : public StorageDev {
private:
    int rpm; // Скорость вращения

public:
    HDD();
    HDD(string n, string m, double p, int c, int r);
    HDD(const HDD& other); // Конструктор копирования
    ~HDD() override;

    string myName() const override;
    void input(istream& is) override;
    void print(ostream& os) const override;

    int getRPM() const { return rpm; }
};
 
class FlashD : public StorageDev {
private:
    int usbSpeed;

public:
    FlashD();
    FlashD(string n, string m, double p, int c, int u);
    FlashD(const FlashD& other); // Конструктор копирования
    ~FlashD() override;

    string myName() const override;
    void input(istream& is) override;
    void print(ostream& os) const override;

    int getUsbSpeed() const { return usbSpeed; }
};

// Класс-контейнер Shop (Магазин)
class Shop {
private:
    vector<StorageDev*> list; // Массив указателей на абстрактный класс

public:
    Shop();
    ~Shop();

    void addDevice(StorageDev* dev);
    void showAll() const;

    void showByCapacityRange(int min, int max) const; // Поиск по диапазону
    void showStats() const;                           // Статистика (емкость/цена)
    void sortByCapacity();                            // Сортировка по объему

    void saveToFile(const string& filename);
    void loadFromFile(const string& filename);
};

#endif