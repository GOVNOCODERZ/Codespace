import csv
import os

class Tour:
    """Класс, описывающий отдельную туристическую путевку."""
    def __init__(self, agency, name, country, duration, price):
        self.__agency = agency
        self.__name = name
        self.__country = country
        self.__duration = int(duration)
        self.__price = float(price)

    # Свойства для доступа к защищенным полям
    @property
    def agency(self): return self.__agency
    @property
    def name(self): return self.__name
    @property
    def country(self): return self.__country
    @property
    def duration(self): return self.__duration
    @property
    def price(self): return self.__price

    @classmethod
    def from_keyboard(cls):
        """Создание объекта через ввод данных с консоли."""
        print("\n--- Добавление нового тура ---")
        agency = input("Введите название агентства: ")
        name = input("Введите название тура: ")
        country = input("Введите страну: ")
        while True:
            try:
                duration = int(input("Длительность (дней): "))
                price = float(input("Цена (руб): "))
                if duration <= 0 or price < 0:
                    print("Числа должны быть положительными.")
                    continue
                break
            except ValueError:
                print("Ошибка! Введите корректные числовые значения.")
        return cls(agency, name, country, duration, price)

    def __str__(self):
        return (f"[{self.agency}] Тур '{self.name}' в {self.country} "
                f"| {self.duration} дн. | {self.price:.2f} руб.")

    # --- Методы проверки условий ---
    
    def check_duration_gt(self, days):
        """Проверяет, превышает ли длительность заданное число дней."""
        return self.__duration > days

    def check_agency_and_duration(self, target_agency, days):
        """Проверяет соответствие агентства и минимальной длительности."""
        return (self.__agency.lower() == target_agency.lower() and 
                self.__duration > days)

    def check_country_and_price(self, target_country, min_p, max_p):
        """Проверяет страну и вхождение цены в диапазон."""
        return (self.__country.lower() == target_country.lower() and 
                min_p <= self.__price <= max_p)


class TourManager:
    """Класс для управления списком туров."""
    def __init__(self):
        self.__tours = []

    def add_tour(self, tour):
        if isinstance(tour, Tour):
            self.__tours.append(tour)

    def fill_from_keyboard(self):
        try:
            count = int(input("Сколько туров добавить? "))
            for _ in range(count):
                self.add_tour(Tour.from_keyboard())
            print("Туры успешно добавлены.")
        except ValueError:
            print("Ошибка ввода количества.")

    def load_from_file(self, filename):
        if not os.path.exists(filename):
            print(f"Файл {filename} не найден.")
            return
        
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.reader(f)
                count = 0
                self.__tours = [] 
                for row in reader:
                    if len(row) == 5:
                        self.add_tour(Tour(*row))
                        count += 1
            print(f"Загружено записей: {count}")
        except Exception as e:
            print(f"Ошибка чтения файла: {e}")

    def save_to_file(self, filename):
        try:
            with open(filename, 'w', encoding='utf-8', newline='') as f:
                writer = csv.writer(f)
                for t in self.__tours:
                    writer.writerow([t.agency, t.name, t.country, t.duration, t.price])
            print(f"Данные успешно сохранены в файл {filename}")
        except Exception as e:
            print(f"Ошибка записи в файл: {e}")

    def display(self):
        if not self.__tours:
            print("Список туров пуст.")
        else:
            print(f"\n--- Список туров ({len(self.__tours)}) ---")
            for i, t in enumerate(self.__tours, 1):
                print(f"{i}. {t}")

    def sort_by_duration(self):
        self.__tours.sort(key=lambda x: x.duration)
        print("Список отсортирован по длительности тура.")

    # --- Методы поиска ---
    
    def find_long_tours(self, min_days):
        result = TourManager()
        for t in self.__tours:
            if t.check_duration_gt(min_days):
                result.add_tour(t)
        return result

    def find_by_agency_duration(self, agency, min_days):
        result = TourManager()
        for t in self.__tours:
            if t.check_agency_and_duration(agency, min_days):
                result.add_tour(t)
        return result

    def find_by_country_price(self, country, min_p, max_p):
        result = TourManager()
        for t in self.__tours:
            if t.check_country_and_price(country, min_p, max_p):
                result.add_tour(t)
        return result

    def is_empty(self):
        return len(self.__tours) == 0
    
    # Метод для доступа к списку (используется для получения длины при выводе)
    def get_count(self):
        return len(self.__tours)