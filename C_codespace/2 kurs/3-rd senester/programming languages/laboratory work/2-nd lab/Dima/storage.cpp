#include "storage.h"

// --- StorageDev ---
StorageDev::StorageDev() : name("None"), manufacturer("None"), price(0), capacity(0) {}
StorageDev::StorageDev(string n, string m, double p, int c) 
    : name(n), manufacturer(m), price(p), capacity(c) {}
StorageDev::~StorageDev() {}

void StorageDev::input(istream& is) {
    if (&is == &cin) cout << "  Название: ";
    is >> name;
    if (&is == &cin) cout << "  Фирма: ";
    is >> manufacturer;
    if (&is == &cin) cout << "  Цена: ";
    is >> price;
    if (&is == &cin) cout << "  Емкость: ";
    is >> capacity;
}

void StorageDev::print(ostream& os) const {
    if (&os == &cout) {
        os << "Фирма: " << manufacturer << " | Название: " << name 
           << " | Цена: " << price << " | Объем: " << capacity;
    } else {
        // Формат для файла: Название Фирма Цена Емкость (без меток)
        os << name << " " << manufacturer << " " << price << " " << capacity;
    }
}

ostream& operator<<(ostream& os, const StorageDev& obj) {
    obj.print(os);
    return os;
}

istream& operator>>(istream& is, StorageDev& obj) {
    obj.input(is);
    return is;
}

HDD::HDD() : StorageDev(), rpm(5400) {}
HDD::HDD(string n, string m, double p, int c, int r) : StorageDev(n, m, p, c), rpm(r) {}

void HDD::input(istream& is) {
    StorageDev::input(is);
    if (&is == &cin) cout << "  RPM: ";
    is >> rpm;
}

void HDD::print(ostream& os) const {
    if (&os == &cout) {
        os << "[HDD] ";
        StorageDev::print(os);
        os << " | RPM: " << rpm;
    } else {
        // В файл сначала пишем тип, потом базовые поля, потом специфичное поле
        os << "HDD ";
        StorageDev::print(os);
        os << " " << rpm;
    }
}

FlashD::FlashD() : StorageDev(), usbSpeed(100) {}
FlashD::FlashD(string n, string m, double p, int c, int u) : StorageDev(n, m, p, c), usbSpeed(u) {}

void FlashD::input(istream& is) {
    StorageDev::input(is);
    if (&is == &cin) cout << "  Скорость USB: ";
    is >> usbSpeed;
}

void FlashD::print(ostream& os) const {
    if (&os == &cout) {
        os << "[Flash] ";
        StorageDev::print(os);
        os << " | Скорость: " << usbSpeed;
    } else {
        os << "Flash ";
        StorageDev::print(os);
        os << " " << usbSpeed;
    }
}

Shop::Shop() {}
Shop::~Shop() {
    for (auto p : list) delete p;
}

void Shop::addDevice(StorageDev* dev) {
    list.push_back(dev);
}

void Shop::showAll() const {
    if (list.empty()) { cout << "Список пуст." << endl; return; }
    for (const auto& dev : list) cout << *dev << endl;
}

void Shop::showByCapacityRange(int min, int max) const {
    for (const auto& dev : list) {
        if (dev->getCapacity() >= min && dev->getCapacity() <= max) cout << *dev << endl;
    }
}

void Shop::showStats() const {
    if (list.empty()) return;
    StorageDev *minObj = list[0], *maxObj = list[0];
    for (auto dev : list) {
        if (dev->getPrice() > 0) {
            double ratio = dev->getCapacity() / dev->getPrice();
            if (ratio < (minObj->getCapacity() / minObj->getPrice())) minObj = dev;
            if (ratio > (maxObj->getCapacity() / maxObj->getPrice())) maxObj = dev;
        }
    }
    cout << "Лучшая цена/объем: " << maxObj->getName() << endl;
    cout << "Худшая цена/объем: " << minObj->getName() << endl;
}

void Shop::sortByCapacity() {
    sort(list.begin(), list.end(), [](StorageDev* a, StorageDev* b) {
        return a->getCapacity() < b->getCapacity();
    });
}

void Shop::saveToFile(const string& filename) {
    ofstream fout(filename);
    if (!fout) return;
    for (const auto& dev : list) {
        fout << *dev << endl; // Использует перегрузку << и логику print для файла
    }
    fout.close();
    cout << "Данные сохранены." << endl;
}

void Shop::loadFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin) return;
    for (auto p : list) delete p;
    list.clear();

    string type;
    while (fin >> type) {
        StorageDev* dev = nullptr;
        if (type == "HDD") dev = new HDD();
        else if (type == "Flash") dev = new FlashD();

        if (dev) {
            fin >> *dev; // Использует перегрузку >> и логику input для файла
            list.push_back(dev);
        }
    }
    fin.close();
    cout << "Данные загружены." << endl;
}