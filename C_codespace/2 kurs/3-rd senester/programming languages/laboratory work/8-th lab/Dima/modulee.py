# models.py
import csv

class Tour:
    """Описывает одну туристическую путевку."""
    def __init__(self, agency, name, country, duration, price):
        self.__agency = agency
        self.__name = name
        self.__country = country
        self.__duration = int(duration)
        self.__price = float(price)

    @property
    def agency(self):
        return self.__agency

    @property
    def name(self):
        return self.__name

    @property
    def country(self):
        return self.__country

    @property
    def duration(self):
        return self.__duration

    @property
    def price(self):
        return self.__price

    def __str__(self):
        """Возвращает строковое представление объекта."""
        return (f"Агентство: {self.agency}, Тур: \"{self.name}\", "
                f"Страна: {self.country}, Длительность: {self.duration} дней, "
                f"Цена: {self.price:.2f} руб.")

    @classmethod
    def from_keyboard(cls):
        """Создает объект тура из данных, введенных с клавиатуры."""
        agency = input("Название агентства: ")
        name = input("Название тура: ")
        country = input("Страна: ")
        duration = int(input("Длительность (дни): "))
        price = float(input("Цена: "))
        return cls(agency, name, country, duration, price)

    def is_duration_over(self, days):
        """Проверка: длительность больше заданного значения?"""
        return self.duration > days

    def is_country(self, country_name):
        """Проверка: страна соответствует заданной?"""
        return self.country.lower() == country_name.lower()

    def is_price_in(self, min_price, max_price):
        """Проверка: цена находится в заданном диапазоне?"""
        return min_price <= self.price <= max_price


class TourManager:
    """Управляет списком объектов Tour."""
    def __init__(self):
        self.__tours = []

    def add(self, tour):
        if isinstance(tour, Tour):
            self.__tours.append(tour)

    def fill_keyboard(self, count):
        for _ in range(count):
            print("-" * 10)
            tour = Tour.from_keyboard()
            self.add(tour)

    def load_file(self, filename):
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.reader(f)
                self.__tours = [Tour(*row) for row in reader]
            print(f"Данные из '{filename}' успешно загружены.")
        except FileNotFoundError:
            print(f"Ошибка: файл '{filename}' не найден.")
        except Exception as e:
            print(f"Ошибка при чтении файла: {e}")


    def save_file(self, filename, tours_list=None):
        tours_to_save = tours_list if tours_list is not None else self.__tours
        try:
            with open(filename, 'w', encoding='utf-8', newline='') as f:
                writer = csv.writer(f)
                for tour in tours_to_save:
                    writer.writerow([tour.agency, tour.name, tour.country, tour.duration, tour.price])
            print(f"Данные успешно сохранены в файл '{filename}'.")
        except Exception as e:
            print(f"Ошибка сохранения файла: {e}")

    def display(self, tours_list=None):
        tours_to_show = tours_list if tours_list is not None else self.__tours
        if not tours_to_show:
            print("Список туров пуст.")
            return
        for i, tour in enumerate(tours_to_show, 1):
            print(f"{i}. {tour}")

    def filter_duration(self, days):
        return [tour for tour in self.__tours if tour.is_duration_over(days)]

    def filter_agency_duration(self, agency, days):
        return [tour for tour in self.__tours if tour.agency.lower() == agency.lower() and tour.is_duration_over(days)]

    def filter_country_price(self, country, min_p, max_p):
        return [tour for tour in self.__tours if tour.is_country(country) and tour.is_price_in(min_p, max_p)]

    def sort_duration(self):
        self.__tours.sort(key=lambda tour: tour.duration)
        print("\nСписок отсортирован по длительности.")