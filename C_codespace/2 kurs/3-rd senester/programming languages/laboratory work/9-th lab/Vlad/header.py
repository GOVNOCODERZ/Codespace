import os
from abc import ABC, abstractmethod

class MediaStorage(ABC):
    """
    Абстрактный класс, представляющий портативный накопитель.
    Поля:
    - `name (str)`: Название.
    - `capacity (int)`: Ёмкость.
    - `manufacturer (str)`: Производитель.
    - `price (float)`: Цена.
    """
    def __init__(self, name="", capacity=0, manufacturer="", price=0.0):
        self._name = name.strip()
        self._capacity = capacity
        self._manufacturer = manufacturer.strip()
        self._price = float(price)

    # Геттеры для общих полей
    def get_name(self): return self._name
    def get_capacity(self): return self._capacity
    def get_manufacturer(self): return self._manufacturer
    def get_price(self): return self._price

    # Абстрактные методы для наследования
    @abstractmethod
    def my_name(self):
        pass

    @abstractmethod
    def input_data(self, prompt=""):
        pass

    @abstractmethod
    def input_from_file(self, file_data):
        pass

    @abstractmethod
    def output(self):
        pass

    @abstractmethod
    def to_line(self):
        pass

class HDD(MediaStorage):
    """
    Класс, представляющий Жёсткий диск.
    Дополнительные поля:
    - `rpm (int)`: RPM (обороты в минуту).
    - `interface (str)`: Интерфейс.
    """
    def __init__(self, name="", capacity=0, manufacturer="", price=0.0, rpm=5400, interface="SATA"):
        super().__init__(name, capacity, manufacturer, price)
        self.__rpm = rpm
        self.__interface = interface.strip()

    def get_rpm(self): return self.__rpm
    def get_interface(self): return self.__interface

    def my_name(self): return "HDD"

    def input_data(self, prompt="Введите данные HDD:"):
        print(prompt)
        name = input("Название: ")
        capacity = int(input("Ёмкость (Гб): "))
        manufacturer = input("Производитель: ")
        price = float(input("Цена: "))
        rpm = int(input("RPM: "))
        interface = input("Интерфейс: ")

        self._name = name.strip()
        self._capacity = capacity
        self._manufacturer = manufacturer.strip()
        self._price = float(price)
        self.__rpm = rpm
        self.__interface = interface.strip()

    def input_from_file(self, file_data):
        parts = file_data.strip().split(',')

        name, capacity, manufacturer, price, rpm, interface = parts

        self._name = name.strip()
        self._capacity = int(capacity)
        self._manufacturer = manufacturer.strip()
        self._price = float(price)
        self.__rpm = int(rpm)
        self.__interface = interface.strip()

    def output(self):
        print(f"Тип: {self.my_name()}, Название: {self._name}, Ёмкость: {self._capacity} Гб, "
              f"Производитель: {self._manufacturer}, Цена: {self._price:.2f}, RPM: {self.__rpm}, Интерфейс: {self.__interface}")

    def to_line(self):
        return f"{self._name},{self._capacity},{self._manufacturer},{self._price:.2f},{self.__rpm},{self.__interface}"


class DVD(MediaStorage):
    """
    Класс, представляющий DVD накопитель.
    Дополнительные поля:
    - `type_dvd (str)`: Тип DVD (например, DVD-R, DVD+R, DVD-RW).
    - `speed (int)`: Скорость записи (как число, например, 16x).
    """
    def __init__(self, name="", capacity=0, manufacturer="", price=0.0, type_dvd="DVD-R", speed=16):
        super().__init__(name, capacity, manufacturer, price)
        self.__type_dvd = type_dvd.strip()
        self.__speed = speed

    def get_type_dvd(self): return self.__type_dvd
    def get_speed(self): return self.__speed

    def my_name(self): return "DVD"

    def input_data(self, prompt="Введите данные DVD:"):
        print(prompt)
        name = input("Название: ")
        capacity = int(input("Ёмкость (Мб): ")) # Обычно у DVD в Мб
        manufacturer = input("Производитель: ")
        price = float(input("Цена: "))
        type_dvd = input("Тип DVD (например, DVD-R): ")
        speed = int(input("Скорость записи (число, напр. 16): "))

        self._name = name.strip()
        self._capacity = capacity
        self._manufacturer = manufacturer.strip()
        self._price = float(price)
        self.__type_dvd = type_dvd.strip()
        self.__speed = speed

    def input_from_file(self, file_data):
        parts = file_data.strip().split(',')

        name, capacity, manufacturer, price, type_dvd, speed = parts

        self._name = name.strip()
        self._capacity = int(capacity)
        self._manufacturer = manufacturer.strip()
        self._price = float(price)
        self.__type_dvd = type_dvd.strip()
        self.__speed = int(speed)

    def output(self):
        print(f"Тип: {self.my_name()}, Название: {self._name}, Ёмкость: {self._capacity} Мб, "
              f"Производитель: {self._manufacturer}, Цена: {self._price:.2f}, Тип: {self.__type_dvd}, Скорость: {self.__speed}x")

    def to_line(self):
        return f"{self._name},{self._capacity},{self._manufacturer},{self._price:.2f},{self.__type_dvd},{self.__speed}"


class Ms:
    """
    Класс-контейнер для списка накопителей (экземпляров классов, наследующих от MediaStorage).
    """
    def __init__(self):
        self.storage_devices = []

    def add_device(self, device):
        self.storage_devices.append(device)

    def input_devices_from_console(self):
        n = int(input("Сколько накопителей добавить? "))
        for _ in range(n):
            print("\nВыберите тип накопителя: 1 - HDD, 2 - DVD")
            type_choice = input("Введите 1 или 2: ")
            if type_choice == '1':
                d = HDD()
            elif type_choice == '2':
                d = DVD()
            else:
                print("Неверный выбор типа. Пропускаем.")
                continue

            d.input_data()
            self.add_device(d)
            print("Накопитель добавлен.")

    def load_from_file(self, filename):
        if not os.path.exists(filename):
            print(f"Файл {filename} не существует. Создаю пустой список.")
            self.storage_devices = []
            return

        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        self.storage_devices = []
        for line in lines:
            if line.strip():
                parts = line.strip().split(',', 1) # Разделить только по первому запятой
                if len(parts) != 2:
                    print(f"Пропускаю строку с неверным форматом: {line}")
                    continue
                device_type, data_part = parts
                if device_type == "HDD":
                    d = HDD()
                elif device_type == "DVD":
                    d = DVD()
                else:
                    print(f"Неизвестный тип устройства: {device_type}. Пропускаю строку: {line}")
                    continue
                d.input_from_file(data_part)
                self.add_device(d)
        print(f"Данные загружены из {filename}")

    def save_to_file(self, filename):
        with open(filename, 'w', encoding='utf-8') as f:
            for device in self.storage_devices:
                f.write(device.my_name() + ',' + device.to_line() + "\n")
        print(f"Данные сохранены в {filename}")

    def display_devices(self):
        if not self.storage_devices:
            print("Список накопителей пуст.")
            return
        for i, device in enumerate(self.storage_devices):
            print(f"--- Накопитель {i+1} ---")
            device.output()

    def select_by_capacity_range(self, min_cap, max_cap):
        return [d for d in self.storage_devices if min_cap <= d.get_capacity() <= max_cap]

    def select_by_type(self, device_type):
        return [d for d in self.storage_devices if d.my_name() == device_type]

    def get_stats_by_type(self):
        stats = {}
        for device in self.storage_devices:
            name = device.my_name()
            if name not in stats:
                stats[name] = {'count': 0, 'total_price': 0.0}
            stats[name]['count'] += 1
            stats[name]['total_price'] += device.get_price()

        result_str = "\n--- Статистика по типам накопителей ---\n"
        for device_type, data in stats.items():
            avg_price = data['total_price'] / data['count'] if data['count'] > 0 else 0
            result_str += f"Тип: {device_type}\n"
            result_str += f"  Количество: {data['count']}\n"
            result_str += f"  Общая цена: {data['total_price']:.2f}\n"
            result_str += f"  Средняя цена: {avg_price:.2f}\n"
        return result_str

    def sort_by_capacity(self):
        self.storage_devices.sort(key=lambda d: d.get_capacity())