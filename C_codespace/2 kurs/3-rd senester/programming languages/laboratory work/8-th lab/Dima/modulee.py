import csv

class Tour:
    """Описывает одну туристическую путевку."""
    def __init__(self, agency, name, country, duration, price):
        self.__agency = agency
        self.__name = name
        self.__country = country
        self.__duration = int(duration)
        self.__price = float(price)

    # --- Свойства (Getters) ---
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

    def __str__(self):
        return (f"Агентство: {self.agency}, Тур: \"{self.name}\", "
                f"Страна: {self.country}, Длительность: {self.duration} дней, "
                f"Цена: {self.price:.2f} руб.")

    @classmethod
    def from_keyboard(cls):
        print("--- Ввод данных тура ---")
        agency = input("Название агентства: ")
        name = input("Название тура: ")
        country = input("Страна: ")
        while True:
            try:
                duration = int(input("Длительность (дни): "))
                price = float(input("Цена: "))
                break
            except ValueError:
                print("Ошибка: введите числа для цены и длительности.")
        return cls(agency, name, country, duration, price)

    # --- Методы проверки ---
    def is_duration_over(self, days):
        return self.duration > days

    def is_country(self, country_name):
        return self.country.lower() == country_name.lower()

    def is_price_in(self, min_price, max_price):
        return min_price <= self.price <= max_price

    def is_agency(self, agency_name):
        return self.agency.lower() == agency_name.lower()


class TourManager:
    """Управляет списком объектов Tour."""
    def __init__(self):
        self.__tours = []

    def add(self, tour):
        if isinstance(tour, Tour):
            self.__tours.append(tour)

    def fill_keyboard(self, count):
        for _ in range(count):
            self.add(Tour.from_keyboard())
        print(f"Добавлено {count} туров.")

    def load_file(self, filename):
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                reader = csv.reader(f)
                count = 0
                self.__tours = [] # Очистка перед загрузкой
                for row in reader:
                    if row:
                        self.__tours.append(Tour(*row))
                        count += 1
            print(f"Загружено {count} туров.")
        except FileNotFoundError:
            print(f"Файл '{filename}' не найден.")
        except Exception as e:
            print(f"Ошибка чтения: {e}")

    def save_file(self, filename):
        try:
            with open(filename, 'w', encoding='utf-8', newline='') as f:
                writer = csv.writer(f)
                for tour in self.__tours:
                    writer.writerow([tour.agency, tour.name, tour.country, tour.duration, tour.price])
            print(f"Сохранено {len(self.__tours)} туров в файл '{filename}'.")
        except Exception as e:
            print(f"Ошибка записи: {e}")

    def display(self):
        if not self.__tours:
            print("Список пуст.")
            return
        print(f"\n--- Список туров ({len(self.__tours)} шт.) ---")
        for i, tour in enumerate(self.__tours, 1):
            print(f"{i}. {tour}")

    def sort_duration(self):
        self.__tours.sort(key=lambda t: t.duration)
        print("Список отсортирован по длительности.")

    def is_empty(self):
        return len(self.__tours) == 0
    
    # а) Список туров с длительностью больше указанного количества дней
    def filter_duration(self, days):
        res = TourManager()
        res.__tours = [t for t in self.__tours if t.is_duration_over(days)]
        return res

    # б) Список туров указанного агентства с длительностью больше Х дней
    def filter_agency_duration(self, agency, days):
        res = TourManager()
        res.__tours = [t for t in self.__tours 
                       if t.is_agency(agency) and t.is_duration_over(days)]
        return res

    # в) Список туров в страну, цена в диапазоне
    def filter_country_price(self, country, min_p, max_p):
        res = TourManager()
        res.__tours = [t for t in self.__tours 
                       if t.is_country(country) and t.is_price_in(min_p, max_p)]
        return res

def process_search_result(result_manager):
    if result_manager.is_empty():
        print("По вашему запросу ничего не найдено.")
        return

    print(f"\nНайдено туров: {len(result_manager._TourManager__tours)}") # Доступ к списку для подсчета
    
    while True:
        print("\nЧто сделать с найденными данными?")
        print("1. Вывести на экран")
        print("2. Сохранить в отдельный файл")
        print("0. Вернуться в главное меню (ничего не делать)")
        
        choice = input("выбор: ")
        
        if choice == "1":
            result_manager.display()
        elif choice == "2":
            fname = input("Введите имя файла для сохранения: ")
            result_manager.save_file(fname)
        elif choice == "0":
            break
        else:
            print("Неверный ввод.")