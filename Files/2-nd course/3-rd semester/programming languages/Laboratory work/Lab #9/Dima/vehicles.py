import csv
from abc import ABC, abstractmethod

class Vehicle(ABC):
    """
    Абстрактный базовый класс Транспортное средство.
    """
    def __init__(self, name, capacity, volume, delivery_time):
        self._name = name
        self._capacity = float(capacity)      # Грузоподъемность (тонн)
        self._volume = float(volume)          # Объем груза (м3)
        self._delivery_time = int(delivery_time) # Срок доставки (часов)

    @property
    def name(self): return self._name
    @property
    def capacity(self): return self._capacity
    @property
    def volume(self): return self._volume
    @property
    def delivery_time(self): return self._delivery_time

    @abstractmethod
    def my_name(self):
        """Возвращает строковое имя типа транспортного средства."""
        pass

    @abstractmethod
    def _get_specific_str(self):
        """Возвращает строку со специфичными параметрами для вывода."""
        pass
    
    @abstractmethod
    def _get_specific_data(self):
        """Возвращает список специфичных полей для сохранения в файл."""
        pass

    def __str__(self):
        base_info = (f"[{self.my_name()}] {self._name}: "
                     f"Груз {self._capacity}т / {self._volume}м3, "
                     f"Время: {self._delivery_time}ч")
        return base_info + self._get_specific_str()

    def get_record(self):
        """Формирует список данных для записи в CSV."""
        return [self.my_name(), self._name, self._capacity, 
                self._volume, self._delivery_time] + self._get_specific_data()

    @staticmethod
    def _get_base_input():
        """Вспомогательный метод для ввода общих данных."""
        name = input("Название транспорта: ")
        while True:
            try:
                cap = float(input("Грузоподъемность (т): "))
                vol = float(input("Объем груза (м3): "))
                time = int(input("Срок доставки (ч): "))
                return name, cap, vol, time
            except ValueError:
                print("Ошибка: введите корректные числовые значения.")


class Train(Vehicle):
    """Класс Поезд."""
    def __init__(self, name, capacity, volume, delivery_time, wagons):
        super().__init__(name, capacity, volume, delivery_time)
        self.__wagons = int(wagons)

    def my_name(self):
        return "Train"

    def _get_specific_str(self):
        return f", Вагонов: {self.__wagons}"

    def _get_specific_data(self):
        return [self.__wagons]

    @classmethod
    def from_keyboard(cls):
        print("\n--- Добавление Поезда ---")
        base_data = cls._get_base_input()
        while True:
            try:
                wagons = int(input("Количество вагонов: "))
                break
            except ValueError:
                print("Введите целое число.")
        return cls(*base_data, wagons)


class Airplane(Vehicle):
    """Класс Самолет."""
    def __init__(self, name, capacity, volume, delivery_time, max_altitude):
        super().__init__(name, capacity, volume, delivery_time)
        self.__max_altitude = int(max_altitude)

    def my_name(self):
        return "Airplane"

    def _get_specific_str(self):
        return f", Высота: {self.__max_altitude}м"

    def _get_specific_data(self):
        return [self.__max_altitude]

    @classmethod
    def from_keyboard(cls):
        print("\n--- Добавление Самолета ---")
        base_data = cls._get_base_input()
        while True:
            try:
                alt = int(input("Макс. высота (м): "))
                break
            except ValueError:
                print("Введите целое число.")
        return cls(*base_data, alt)


class TransportCompany:
    """Класс-контейнер для управления парком техники."""
    def __init__(self):
        self.__vehicles = [] 

    def add(self, vehicle):
        if isinstance(vehicle, Vehicle):
            self.__vehicles.append(vehicle)

    def is_empty(self):
        return len(self.__vehicles) == 0

    def display(self):
        if self.is_empty():
            print("Список транспорта пуст.")
            return
        print(f"\n--- Парк техники ({len(self.__vehicles)} шт.) ---")
        for i, v in enumerate(self.__vehicles, 1):
            print(f"{i}. {v}")

    def save_file(self, filename):
        if self.is_empty():
            print("Нечего сохранять.")
            return
        try:
            with open(filename, 'w', encoding='utf-8', newline='') as f:
                writer = csv.writer(f)
                for v in self.__vehicles:
                    writer.writerow(v.get_record())
            print(f"Данные успешно сохранены в '{filename}'.")
        except Exception as e:
            print(f"Ошибка сохранения: {e}")

    def load_file(self, filename):
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.reader(f)
                count = 0
                self.__vehicles = []
                for row in reader:
                    if not row: continue
                    type_obj = row[0]
                    args = row[1:]
                    
                    if type_obj == "Train":
                        self.add(Train(*args))
                        count += 1
                    elif type_obj == "Airplane":
                        self.add(Airplane(*args))
                        count += 1
            print(f"Загружено {count} объектов из '{filename}'.")
        except FileNotFoundError:
            print("Файл не найден.")
        except Exception as e:
            print(f"Ошибка загрузки: {e}")

    def filter_time_below(self, limit_time):
        """Возвращает новый объект TransportCompany с техникой, срок доставки которой ниже лимита."""
        res = TransportCompany()
        # Прямой доступ к списку внутри класса допустим
        res._TransportCompany__vehicles = [v for v in self.__vehicles if v.delivery_time < limit_time]
        return res

    def filter_capacity_above(self, limit_cap):
        """Возвращает новый объект TransportCompany с техникой, грузоподъемность которой выше лимита."""
        res = TransportCompany()
        res._TransportCompany__vehicles = [v for v in self.__vehicles if v.capacity > limit_cap]
        return res

    def sort_by_volume(self):
        self.__vehicles.sort(key=lambda v: v.volume)
        print("Список отсортирован по возрастанию объема груза.")
    
    def get_count(self):
        return len(self.__vehicles)