import os
from abc import ABC, abstractmethod

class MediaStorage(ABC):
    """
    Абстрактный класс, представляющий собой Накопитель памяти.
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
    def my_name(self):
        return type(self).__name__

    @abstractmethod
    def stats_to_line(self):
        pass
    
    def data_output(self):
        return (f"Тип: {self.my_name()}, "
                f"Название: {self._name}, "
                f"Ёмкость: {self._capacity} ГБ, "
                f"Производитель: {self._manufacturer}, "
                f"Цена: {self._price:.2f}, руб.")
        
class PortableHDD(MediaStorage):
    """
    Класс, представляющий Жёсткий диск.
    Дополнительные поля:
    - `rpm (int)`: RPM (обороты в минуту).
    """
    def __init__(self, name="", capacity=0, manufacturer="", price=0.0, rpm=0):
        super().__init__(name, capacity, manufacturer, price)
        self.__rpm = rpm

    def get_rpm(self): return self.__rpm

    def data_output(self):
        return (f"{super().data_output()}, "
                f"RPM: {self.__rpm}, ")

    def stats_to_line(self):
        return f"{self._name},{self._capacity},{self._manufacturer},{self._price:.2f},{self.__rpm}"


class DVD(MediaStorage):
    """
    Класс, представляющий DVD накопитель.
    Дополнительные поля:
    - `type_dvd (str)`: Тип DVD (например, DVD-R, DVD+R, DVD-RW).
    """
    def __init__(self, name="", capacity=0, manufacturer="", price=0.0, type_dvd="DVD-R"):
        super().__init__(name, capacity, manufacturer, price)
        self.__type_dvd = type_dvd.strip()

    def get_type_dvd(self): return self.__type_dvd

    def data_output(self):
        return (f"{super().data_output()}, "
                f"Тип: {self.__type_dvd}, ")

    def stats_to_line(self):
        return f"{self._name},{self._capacity},{self._manufacturer},{self._price:.2f},{self.__type_dvd}"


class Shop:
    """
    Класс-контейнер для списка накопителей (экземпляров классов, наследующихся от MediaStorage).
    """
    def __init__(self):
        self.__storage_devices = []

    def add_device(self, device):
        self.__storage_devices.append(device)

    def load_from_file(self, filename):
        if not os.path.exists(filename):
            print(f"Файл {filename} не найден.")
            return

        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        self.__storage_devices = [] # Предварительно очистим список
        for line in lines:
            parts = line.strip().split(',')
            device_type, name, capacity, manufacturer, price, extra = parts
            match device_type:
                case "PortableHDD":
                    self.__storage_devices.append(PortableHDD(name, capacity, manufacturer, price, extra))
                case "DVD":
                    self.__storage_devices.append(DVD(name, capacity, manufacturer, price, extra))
                case _:
                    print(f"Неизвестный тип устройства: {device_type}. Пропускаю строку: {line}")
        print(f"Данные загружены из {filename}.")

    def save_to_file(self, filename):
        with open(filename, 'w', encoding='utf-8') as f:
            for device in self.__storage_devices:
                f.write(device.my_name() + ',' + device.stats_to_line() + "\n")
        print(f"Данные сохранены в {filename}")

    def display_devices(self):
        if not self.__storage_devices:
            print("Список накопителей пуст.")
            return
        for i, device in enumerate(self.__storage_devices):
            print(f"--- Накопитель {i+1} ---")
            device.data_output()
            
    # Выборка по ёмкости в диапазоне
    def select_by_capacity_range(self, min_cap, max_cap):
        return [d for d in self.__storage_devices if min_cap <= d.get_capacity() <= max_cap]

    # Вывод статистической сводки
    def get_stats_by_type(self):
        stats = {}
        # Собираем статистику
        for device in self.__storage_devices:
            name = device.my_name()
            if name not in stats:
                stats[name] = {'count': 0, 'total_price': 0.0}
            stats[name]['count'] += 1
            stats[name]['total_price'] += device.get_price()

        result_str = "\n--- Статистическая сводка ---\n"
        for device_type, data in stats.items():
            avg_price = data['total_price'] / data['count']
            result_str += f"Тип: {device_type}\n"
            result_str += f"Количество: {data['count']}\n"
            result_str += f"Общая цена: {data['total_price']:.2f}\n"
            result_str += f"Средняя цена: {avg_price:.2f}\n"
        return result_str

    # Сортировка массива по возрастанию цены
    def sort_by_price(self):
        self.__storage_devices.sort(key=lambda d: d.get_price())