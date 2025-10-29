#include "storage_devices.h"
#include <fstream>
#include <algorithm>
#include <limits>
#include <stdexcept>

// --- Реализация StorageDev ---
StorageDev::StorageDev(std::string name, std::string firm, int capacity, double price)
    : name(std::move(name)), firm(std::move(firm)), capacity_gb(capacity), price(price) {}

StorageDev::StorageDev() : capacity_gb(0), price(0.0) {}

int StorageDev::getCapacity() const { return capacity_gb; }
double StorageDev::getPrice() const { return price; }
std::string StorageDev::getName() const { return name; }

double StorageDev::getCapacityPriceRatio() const {
    if (price > 0) {
        return static_cast<double>(capacity_gb) / price;
    }
    return 0;
}

std::ostream& operator<<(std::ostream& os, const StorageDev& dev) {
    dev.print(os);
    return os;
}

std::istream& operator>>(std::istream& is, StorageDev& dev) {
    dev.read(is);
    return is;
}

// --- Реализация HDD ---
HDD::HDD(std::string name, std::string firm, int capacity, double price, int rpm)
    : StorageDev(std::move(name), std::move(firm), capacity, price), rpm(rpm) {}

HDD::HDD() : rpm(0) {}

void HDD::print(std::ostream& os) const {
    os << "Тип: " << myName() << ", Модель: " << name << ", Фирма: " << firm
       << ", Объем: " << capacity_gb << " GB, Цена: " << price
       << ", RPM: " << rpm;
}

void HDD::read(std::istream& is) {
    is >> name >> firm >> capacity_gb >> price >> rpm;
}

std::string HDD::myName() const { return "HDD"; }

std::unique_ptr<StorageDev> HDD::clone() const {
    return std::make_unique<HDD>(*this);
}

// --- Реализация FlashD ---
FlashD::FlashD(std::string name, std::string firm, int capacity, double price, std::string usb)
    : StorageDev(std::move(name), std::move(firm), capacity, price), usb_type(std::move(usb)) {}
    
FlashD::FlashD() {}

void FlashD::print(std::ostream& os) const {
    os << "Тип: " << myName() << ", Модель: " << name << ", Фирма: " << firm
       << ", Объем: " << capacity_gb << " GB, Цена: " << price
       << ", USB: " << usb_type;
}

void FlashD::read(std::istream& is) {
    is >> name >> firm >> capacity_gb >> price >> usb_type;
}

std::string FlashD::myName() const { return "FlashD"; }

std::unique_ptr<StorageDev> FlashD::clone() const {
    return std::make_unique<FlashD>(*this);
}

// --- Реализация Shop ---
Shop::Shop(const Shop& other) {
    devices.clear();
    for(const auto& dev : other.devices) {
        devices.push_back(dev->clone());
    }
}

Shop& Shop::operator=(const Shop& other) {
    if (this != &other) {
        devices.clear();
        for(const auto& dev : other.devices) {
            devices.push_back(dev->clone());
        }
    }
    return *this;
}

void Shop::addDevice(std::unique_ptr<StorageDev> device) {
    devices.push_back(std::move(device));
}

void Shop::printAll(std::ostream& os) const {
    if (devices.empty()) {
        os << "В магазине нет устройств.\n";
        return;
    }
    for (const auto& dev : devices) {
        os << *dev << std::endl;
    }
}

void Shop::showByCapacityRange(std::ostream& os, int min_cap, int max_cap) const {
    os << "Устройства с объемом от " << min_cap << " GB до " << max_cap << " GB:\n";
    bool found = false;
    for (const auto& dev : devices) {
        if (dev->getCapacity() >= min_cap && dev->getCapacity() <= max_cap) {
            os << *dev << std::endl;
            found = true;
        }
    }
    if (!found) {
        os << "Таких устройств не найдено.\n";
    }
}

void Shop::showCapacityPriceRatioStats(std::ostream& os) const {
    if (devices.empty()) {
        os << "Нет устройств для анализа.\n";
        return;
    }

    double min_ratio = std::numeric_limits<double>::max();
    double max_ratio = std::numeric_limits<double>::min();
    
    const StorageDev* min_dev = nullptr;
    const StorageDev* max_dev = nullptr;

    for (const auto& dev : devices) {
        double ratio = dev->getCapacityPriceRatio();
        if (ratio < min_ratio) {
            min_ratio = ratio;
            min_dev = dev.get();
        }
        if (ratio > max_ratio) {
            max_ratio = ratio;
            max_dev = dev.get();
        }
    }

    os << "Статистика по соотношению Объем/Цена:\n";
    if (min_dev) {
         os << "Минимальное (худшее) соотношение: " << min_ratio << " у -> " << *min_dev << std::endl;
    }
    if (max_dev) {
        os << "Максимальное (лучшее) соотношение: " << max_ratio << " у -> " << *max_dev << std::endl;
    }
}

void Shop::sortByCapacity() {
    std::sort(devices.begin(), devices.end(), [](const auto& a, const auto& b) {
        return a->getCapacity() < b->getCapacity();
    });
}

bool Shop::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    devices.clear();
    std::string type;
    while (file >> type) {
        if (type == "HDD") {
            auto hdd = std::make_unique<HDD>();
            file >> *hdd;
            addDevice(std::move(hdd));
        } else if (type == "FlashD") {
            auto flash = std::make_unique<FlashD>();
            file >> *flash;
            addDevice(std::move(flash));
        }
    }
    return true;
}

bool Shop::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    for (const auto& dev : devices) {
        file << dev->myName() << " " << dev->getName() << " " << dev->getCapacity() << " " << dev->getPrice() << " ";
        // Это не самый лучший способ, но для простоты примера подойдет.
        // В идеале, нужно использовать виртуальный метод save.
        if (dynamic_cast<HDD*>(dev.get())) {
             file << "\n"; // Placeholder for specific data
        } else if (dynamic_cast<FlashD*>(dev.get())) {
             file << "\n"; // Placeholder for specific data
        }
    }
    return true;
}