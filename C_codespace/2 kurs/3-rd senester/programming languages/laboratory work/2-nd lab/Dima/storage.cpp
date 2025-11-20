#include "storage.h"

StorageDev::StorageDev() 
    : name("None"), manufacturer("None"), price(0), capacity(0) {}

StorageDev::StorageDev(string n, string m, double p, int c) 
    : name(n), manufacturer(m), price(p), capacity(c) {}

StorageDev::StorageDev(const StorageDev& other) 
    : name(other.name), manufacturer(other.manufacturer), price(other.price), capacity(other.capacity) {}

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
    os << "Фирма: " << manufacturer 
       << " | Название: " << name 
       << " | Цена: " << price 
       << " | Объем: " << capacity;
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

HDD::HDD(string n, string m, double p, int c, int r) 
    : StorageDev(n, m, p, c), rpm(r) {}

HDD::HDD(const HDD& other) : StorageDev(other), rpm(other.rpm) {}

HDD::~HDD() {}

string HDD::myName() const { return "HDD"; }

void HDD::input(istream& is) {
    StorageDev::input(is);
    if (&is == &cin) cout << "  RPM: ";
    is >> rpm;
}

void HDD::print(ostream& os) const {
    os << "[HDD] ";
    StorageDev::print(os);
    os << " | RPM: " << rpm;
}

FlashD::FlashD() : StorageDev(), usbSpeed(100) {}

FlashD::FlashD(string n, string m, double p, int c, int u) 
    : StorageDev(n, m, p, c), usbSpeed(u) {}

FlashD::FlashD(const FlashD& other) : StorageDev(other), usbSpeed(other.usbSpeed) {}

FlashD::~FlashD() {}

string FlashD::myName() const { return "Flash"; }

void FlashD::input(istream& is) {
    StorageDev::input(is);
    if (&is == &cin) cout << "  Скорость USB: ";
    is >> usbSpeed;
}

void FlashD::print(ostream& os) const {
    os << "[Flash] ";
    StorageDev::print(os);
    os << " | Скорость: " << usbSpeed;
}

Shop::Shop() {}

Shop::~Shop() {
    for (auto p : list) delete p;
    list.clear();
}

void Shop::addDevice(StorageDev* dev) {
    list.push_back(dev);
}

void Shop::showAll() const {
    if (list.empty()) {
        cout << "Список пуст." << endl;
        return;
    }
    cout << "--- Список устройств ---" << endl;
    for (const auto& dev : list) {
        cout << *dev << endl;
    }
}

void Shop::showByCapacityRange(int min, int max) const {
    cout << "--- Фильтр: Емкость от " << min << " до " << max << " ---" << endl;
    bool found = false;
    for (const auto& dev : list) {
        if (dev->getCapacity() >= min && dev->getCapacity() <= max) {
            cout << *dev << endl;
            found = true;
        }
    }
    if (!found) cout << "Устройства не найдены." << endl;
}

void Shop::showStats() const {
    if (list.empty()) return;
    double minRatio = -1, maxRatio = -1;
    StorageDev* minObj = nullptr;
    StorageDev* maxObj = nullptr;

    for (const auto& dev : list) {
        if (dev->getPrice() <= 0) continue;
        double ratio = (double)dev->getCapacity() / dev->getPrice();

        if (minObj == nullptr || ratio < minRatio) {
            minRatio = ratio;
            minObj = dev;
        }
        if (maxObj == nullptr || ratio > maxRatio) {
            maxRatio = ratio;
            maxObj = dev;
        }
    }
    cout << "--- Статистика (Емкость / Цена) ---" << endl;
    if (minObj) cout << "Минимум (" << minRatio << "): " << minObj->getName() << endl;
    if (maxObj) cout << "Максимум (" << maxRatio << "): " << maxObj->getName() << endl;
}

void Shop::sortByCapacity() {
    sort(list.begin(), list.end(), [](StorageDev* a, StorageDev* b) {
        return a->getCapacity() < b->getCapacity();
    });
    cout << "Список отсортирован." << endl;
}

void Shop::saveToFile(const string& filename) {
    ofstream fout(filename);
    if (!fout.is_open()) {
        cout << "Ошибка открытия файла!" << endl;
        return;
    }

    for (const auto& dev : list) {
        // Определяем тип и пишем метку
        // Используем dynamic_cast для доступа к специфичным геттерам
        if (HDD* h = dynamic_cast<HDD*>(dev)) {
            fout << "HDD " 
                 << h->getName() << " " 
                 << h->getManufacturer() << " " 
                 << h->getPrice() << " " 
                 << h->getCapacity() << " " 
                 << h->getRPM() << endl; // Пишем RPM
        } 
        else if (FlashD* f = dynamic_cast<FlashD*>(dev)) {
            fout << "Flash " 
                 << f->getName() << " " 
                 << f->getManufacturer() << " " 
                 << f->getPrice() << " " 
                 << f->getCapacity() << " " 
                 << f->getUsbSpeed() << endl; // Пишем скорость USB
        }
    }
    fout.close();
    cout << "Данные успешно сохранены в " << filename << endl;
}

void Shop::loadFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "Файл не найден." << endl;
        return;
    }

    // Очищаем текущий список перед загрузкой
    for (auto p : list) delete p;
    list.clear();

    string type, name, manuf;
    double price;
    int cap, specParam; // specParam будет или RPM, или Speed

    // Читаем в том же порядке, в котором писали
    while (fin >> type >> name >> manuf >> price >> cap >> specParam) {
        StorageDev* newDev = nullptr;
        
        if (type == "HDD") {
            // Создаем HDD, используя прочитанные данные (specParam = rpm)
            newDev = new HDD(name, manuf, price, cap, specParam);
        } 
        else if (type == "Flash") {
            // Создаем FlashD (specParam = usbSpeed)
            newDev = new FlashD(name, manuf, price, cap, specParam);
        }

        if (newDev) {
            list.push_back(newDev);
        }
    }

    fin.close();
    cout << "Данные загружены из файла." << endl;
}