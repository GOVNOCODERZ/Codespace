#ifndef STORAGE_DEVICES_H
#define STORAGE_DEVICES_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Абстрактный базовый класс для накопителей
class StorageDev {
protected:
    std::string name;
    std::string firm;
    int capacity_gb;
    double price;

public:
    // Конструкторы и деструктор
    StorageDev(std::string name, std::string firm, int capacity, double price);
    StorageDev();
    virtual ~StorageDev() = default; // Виртуальный деструктор для корректного удаления

    // Чистые виртуальные функции для полиморфного ввода/вывода
    virtual void print(std::ostream& os) const = 0;
    virtual void read(std::istream& is) = 0;

    // Виртуальный метод для получения имени класса (демонстрация)
    virtual std::string myName() const = 0;
    
    // Геттеры для общих полей
    int getCapacity() const;
    double getPrice() const;
    std::string getName() const;

    // Виртуальный метод для получения соотношения емкость/цена
    virtual double getCapacityPriceRatio() const;

    // Виртуальный клон-метод для реализации копирования в полиморфном контексте
    virtual std::unique_ptr<StorageDev> clone() const = 0;
};

// Операторы потокового ввода/вывода
std::ostream& operator<<(std::ostream& os, const StorageDev& dev);
std::istream& operator>>(std::istream& is, StorageDev& dev);

// Класс "Жесткий диск" (HDD)
class HDD : public StorageDev {
private:
    int rpm; // Скорость вращения шпинделя

public:
    HDD(std::string name, std::string firm, int capacity, double price, int rpm);
    HDD();

    void print(std::ostream& os) const override;
    void read(std::istream& is) override;
    std::string myName() const override;
    std::unique_ptr<StorageDev> clone() const override;
};

// Класс "Флеш-накопитель"
class FlashD : public StorageDev {
private:
    std::string usb_type;

public:
    FlashD(std::string name, std::string firm, int capacity, double price, std::string usb);
    FlashD();

    void print(std::ostream& os) const override;
    void read(std::istream& is) override;
    std::string myName() const override;
    std::unique_ptr<StorageDev> clone() const override;
};

// Класс "Магазин", управляющий коллекцией накопителей
class Shop {
private:
    std::vector<std::unique_ptr<StorageDev>> devices;

public:
    // Конструктор копирования
    Shop(const Shop& other);
    // Оператор присваивания копированием
    Shop& operator=(const Shop& other);
    // Конструктор по умолчанию
    Shop() = default;

    void addDevice(std::unique_ptr<StorageDev> device);
    
    // Методы для выполнения заданий варианта
    void printAll(std::ostream& os) const;
    void showByCapacityRange(std::ostream& os, int min_cap, int max_cap) const;
    void showCapacityPriceRatioStats(std::ostream& os) const;
    void sortByCapacity();

    // Методы для работы с файлами
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
};

#endif