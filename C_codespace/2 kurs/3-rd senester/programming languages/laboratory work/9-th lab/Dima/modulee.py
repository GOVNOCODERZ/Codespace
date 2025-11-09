# models.py
from abc import ABC, abstractmethod

# Абстрактный базовый класс для всех транспортных средств (Ab).
class Vehicle(ABC):
    def __init__(self, name, capacity, volume, time):
        # Защищенные поля для базовых характеристик.
        self._name = name
        self._capacity = capacity  # Грузоподъемность
        self._volume = volume      # Объем перевозимого груза
        self._time = time          # Срок доставки

    # Метод для получения имени класса.
    def my_name(self):
        return type(self).__name__

    # Абстрактный метод для расчета специфического коэффициента.
    # Должен быть переопределен в дочерних классах.
    @abstractmethod
    def get_specific_info(self):
        pass

    # Свойство для безопасного доступа к имени.
    @property
    def name(self):
        return self._name

    # Свойство для безопасного доступа к грузоподъемности.
    @property
    def capacity(self):
        return self._capacity

    # Свойство для безопасного доступа к объему.
    @property
    def volume(self):
        return self._volume

    # Свойство для безопасного доступа ко времени.
    @property
    def time(self):
        return self._time

    # Строковое представление объекта.
    def __str__(self):
        return (f"Тип: {self.my_name()}, Название: {self._name}, "
                f"Грузоподъемность: {self._capacity} кг, "
                f"Объем груза: {self._volume} м³, Срок: {self._time} ч")

# Класс Поезд, наследник Vehicle.
class Train(Vehicle):
    def __init__(self, name, capacity, volume, time, wagons):
        super().__init__(name, capacity, volume, time)
        # Приватное поле для количества вагонов.
        self.__wagons = wagons

    # Реализация абстрактного метода.
    def get_specific_info(self):
        return f"Количество вагонов: {self.__wagons}"

    # Переопределение строкового представления.
    def __str__(self):
        return f"{super().__str__()}, {self.get_specific_info()}"

# Класс Самолет, наследник Vehicle.
class Airplane(Vehicle):
    def __init__(self, name, capacity, volume, time, flight_range):
        super().__init__(name, capacity, volume, time)
        # Приватное поле для дальности полета.
        self.__flight_range = flight_range

    # Реализация абстрактного метода.
    def get_specific_info(self):
        return f"Дальность полета: {self.__flight_range} км"

    # Переопределение строкового представления.
    def __str__(self):
        return f"{super().__str__()}, {self.get_specific_info()}"

# Класс Транспортная Компания (Ms).
class TransportCompany:
    def __init__(self):
        # Приватное поле для хранения списка транспортных средств.
        self.__vehicles = []

    # Метод для загрузки данных из файла.
    def load_from_file(self, filename):
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                self.__vehicles = [] # Очищаем список перед загрузкой
                for line in f:
                    parts = line.strip().split(',')
                    v_type = parts[0].strip()
                    name = parts[1].strip()
                    capacity = int(parts[2].strip())
                    volume = int(parts[3].strip())
                    time = int(parts[4].strip())
                    spec = int(parts[5].strip())
                    
                    if v_type == 'Train':
                        self.__vehicles.append(Train(name, capacity, volume, time, spec))
                    elif v_type == 'Airplane':
                        self.__vehicles.append(Airplane(name, capacity, volume, time, spec))
            print(f"Данные успешно загружены из файла '{filename}'.")
        except FileNotFoundError:
            print(f"Ошибка: Файл '{filename}' не найден.")
        except Exception as e:
            print(f"Ошибка при чтении файла: {e}")

    # Метод для вывода характеристик объектов.
    def show_all(self):
        if not self.__vehicles:
            print("Список транспортных средств пуст.")
            return
        print("\n--- Полный список транспорта ---")
        for v in self.__vehicles:
            print(v)
        print("---------------------------------")

    # Вывод транспорта со сроком доставки ниже заданного.
    def show_by_delivery_time(self, max_time):
        result = [v for v in self.__vehicles if v.time < max_time]
        if not result:
            print(f"Нет транспорта со сроком доставки меньше {max_time} ч.")
            return
        print(f"\n--- Транспорт со сроком доставки < {max_time} ч ---")
        for v in result:
            print(v)
        print("-----------------------------------------")

    # Вывод транспорта с грузоподъемностью выше заданной.
    def show_by_capacity(self, min_capacity):
        result = [v for v in self.__vehicles if v.capacity > min_capacity]
        if not result:
            print(f"Нет транспорта с грузоподъемностью больше {min_capacity} кг.")
            return
        print(f"\n--- Транспорт с грузоподъемностью > {min_capacity} кг ---")
        for v in result:
            print(v)
        print("-------------------------------------------------")

    # Вывод транспорта, отсортированного по объему груза.
    def show_sorted_by_volume(self):
        if not self.__vehicles:
            print("Список транспортных средств пуст.")
            return
        sorted_vehicles = sorted(self.__vehicles, key=lambda v: v.volume)
        print("\n--- Транспорт, отсортированный по объему груза ---")
        for v in sorted_vehicles:
            print(v)
        print("----------------------------------------------------")