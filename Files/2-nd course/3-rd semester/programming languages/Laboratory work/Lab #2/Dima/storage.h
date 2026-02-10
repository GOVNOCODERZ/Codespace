#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

// Базовый абстрактный класс
class StorageDev {
protected:
    string name;
    string manufacturer;
    double price;
    int capacity;

public:
    StorageDev();
    StorageDev(string n, string m, double p, int c);
    virtual ~StorageDev();

    virtual string myName() const = 0;
    
    // Полиморфные методы ввода/вывода
    virtual void input(istream& is);
    virtual void print(ostream& os) const;

    int getCapacity() const { return capacity; }
    double getPrice() const { return price; }
    string getName() const { return name; }

    friend ostream& operator<<(ostream& os, const StorageDev& obj);
    friend istream& operator>>(istream& is, StorageDev& obj);
};

// Класс HDD
class HDD : public StorageDev {
private:
    int rpm;

public:
    HDD();
    HDD(string n, string m, double p, int c, int r);
    
    string myName() const override { return "HDD"; }
    void input(istream& is) override;
    void print(ostream& os) const override;
};

// Класс Flash-диск
class FlashD : public StorageDev {
private:
    int usbSpeed;

public:
    FlashD();
    FlashD(string n, string m, double p, int c, int u);

    string myName() const override { return "Flash"; }
    void input(istream& is) override;
    void print(ostream& os) const override;
};

// Контейнер Shop
class Shop {
private:
    vector<StorageDev*> list;

public:
    Shop();
    ~Shop();

    void addDevice(StorageDev* dev);
    void showAll() const;
    void showByCapacityRange(int min, int max) const;
    void showStats() const;
    void sortByCapacity();

    void saveToFile(const string& filename);
    void loadFromFile(const string& filename);
};

#endif