import csv
from abc import ABC, abstractmethod

class Vehicle(ABC):
    """
    Абстрактный базовый класс Транспортное средство (Ab).
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
        """Абстрактный метод, возвращает имя класса."""
        pass

    @abstractmethod
    def __str__(self):
        """Абстрактный метод строкового представления."""
        pass
    
    # Виртуальный метод для ввода базовых данных (используется в наследниках)
    @staticmethod
    def _get_base_input():
        name = input("Название транспорта: ")
        while True:
            try:
                cap = float(input("Грузоподъемность (т): "))
                vol = float(input("Объем груза (м3): "))
                time = int(input("Срок доставки (ч): "))
                return name, cap, vol, time
            except ValueError:
                print("Ошибка: введите числовые значения.")


class Train(Vehicle):
    """
    Производный класс Поезд.
    """
    def __init__(self, name, capacity, volume, delivery_time, wagons):
        # Вызов конструктора базового класса
        super().__init__(name, capacity, volume, delivery_time)
        self.__wagons = int(wagons) # Специфичное поле: кол-во вагонов

    @property
    def wagons(self): return self.__wagons

    def my_name(self):
        return "Train"
    
# Сделать один вывод для наследников

    def __str__(self):
        return (f"[{self.my_name()}] {self.name}: Груз {self.capacity}т / {self.volume}м3, "
                f"Время: {self.delivery_time}ч, Вагонов: {self.wagons}")

    @classmethod
    def from_keyboard(cls):
        print("\n--- Добавление Поезда ---")
        base_data = cls._get_base_input() # Получаем данные для родителя
        while True:
            try:
                wagons = int(input("Количество вагонов: "))
                break
            except ValueError:
                print("Введите целое число.")
        # Распаковываем base_data и добавляем специфичные данные
        return cls(*base_data, wagons)


class Airplane(Vehicle):
    """
    Производный класс Самолет.
    Поля должны быть закрытыми (__private).
    """
    def __init__(self, name, capacity, volume, delivery_time, max_altitude):
        super().__init__(name, capacity, volume, delivery_time)
        self.__max_altitude = int(max_altitude) # Специфичное поле: высота полета

    @property
    def max_altitude(self): return self.__max_altitude

    def my_name(self):
        return "Airplane"

    def __str__(self):
        return (f"[{self.my_name()}] {self.name}: Груз {self.capacity}т / {self.volume}м3, "
                f"Время: {self.delivery_time}ч, Высота: {self.max_altitude}м")

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
    """
    Класс Ms (Container). Управляет списком транспортных средств.
    """
    def __init__(self):
        self.__vehicles = [] # Список полиморфных объектов

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
        try:
            with open(filename, 'w', encoding='utf-8', newline='') as f:
                writer = csv.writer(f)
                for v in self.__vehicles:
                    # Сохраняем тип класса первым полем, чтобы знать, как загружать
                    if isinstance(v, Train):
                        
                        # Сделать отдельный метод в базовом классе. Убрать проверку

                        writer.writerow(["Train", v.name, v.capacity, v.volume, v.delivery_time, v.wagons])
                    elif isinstance(v, Airplane):
                        writer.writerow(["Airplane", v.name, v.capacity, v.volume, v.delivery_time, v.max_altitude])
            print(f"Данные сохранены в '{filename}'.")
        except Exception as e:
            print(f"Ошибка сохранения: {e}")

    def load_file(self, filename):
        try:
            self.__vehicles = []
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.reader(f)
                count = 0
                for row in reader:
                    if not row: continue
                    type_obj = row[0]
                    args = row[1:] # Остальные параметры
                    
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

    # 1. Вывод транспорта, срок доставки которого НИЖЕ заданного
    def filter_time_below(self, limit_time):
        res = TransportCompany()
        # Доступ к приватному полю возможен внутри класса
        res.__vehicles = [v for v in self.__vehicles if v.delivery_time < limit_time]
        return res

    # 2. Вывод транспорта с грузоподъемностью ВЫШЕ заданной
    def filter_capacity_above(self, limit_cap):
        res = TransportCompany()
        res.__vehicles = [v for v in self.__vehicles if v.capacity > limit_cap]
        return res

    # 3. Сортировка массива по УВЕЛИЧЕНИЮ объема груза
    def sort_by_volume(self):
        # Сортировка текущего списка in-place
        self.__vehicles.sort(key=lambda v: v.volume)
        print("Список отсортирован по возрастанию объема груза.")

def process_search_result(manager):
    """Меню действий с найденными результатами."""
    if manager.is_empty():
        print("Ничего не найдено по вашему запросу.")
        return

    print(f"Найдено записей: {len(manager._TransportCompany__vehicles)}")
    while True:
        print("\n[Результаты поиска]")
        print("1. Показать на экране")
        print("2. Сохранить в файл")
        print("0. Вернуться назад")
        choice = input("Выбор: ")
        
        if choice == "1":
            manager.display()
        elif choice == "2":
            fname = input("Имя файла: ")
            manager.save_file(fname)
        elif choice == "0":
            break
        else:
            print("Неверный ввод.")
