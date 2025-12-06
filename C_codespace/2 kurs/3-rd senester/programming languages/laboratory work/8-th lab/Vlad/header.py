class House:
    """
    Класс, представляющий квартиру.
    Поля:
    - `address (str)`: Адрес квартиры.
    - `building_type (str)`: Тип здания.
    - `floor (int)`: Этаж.
    - `rooms (int)`: Количество комнат.
    - `area (float)`: Площадь в кв. м.
    """

    def __init__(self, address="", building_type="", floor=1, rooms=1, area=0.0):
        self.__address = address.strip()
        self.__building_type = building_type.strip()
        self.__floor = floor
        self.__rooms = rooms
        self.__area = float(area)

    # Геттеры для полей
    def get_address(self):
        return self.__address

    def get_building_type(self):
        return self.__building_type

    def get_floor(self):
        return self.__floor

    def get_rooms(self):
        return self.__rooms

    def get_area(self):
        return self.__area

    # Методы ввода и вывода
    def input_data(self, prompt="Введите данные о квартире:"):
        """
        Метод ввода данных с клавиатуры.
        """
        print(prompt)
        address = input("Адрес: ")
        building_type = input("Тип здания: ")
        floor = int(input("Этаж: "))
        rooms = int(input("Количество комнат: "))
        area = float(input("Площадь (кв. м.): "))

        self.__address = address.strip()
        self.__building_type = building_type.strip()
        self.__floor = floor
        self.__rooms = rooms
        self.__area = float(area)

    def input_from_file(self, file_data):
        """
        Метод ввода данных из файла.
        """
        # Читаем строку и делим на 5 подстрок
        parts = file_data.strip().split(',')
        if len(parts) != 5:
            raise ValueError("Строка не содержит 5 значений, разделённых запятыми.")

        
        address = parts[0]
        building_type = parts[1]
        floor = int(parts[2])
        rooms = int(parts[3])
        area = float(parts[4])

        self.__address = address.strip()
        self.__building_type = building_type.strip()
        self.__floor = floor
        self.__rooms = rooms
        self.__area = float(area)

    def output(self):
        """
        Метод вывода данных на экран.
        """
        print(f"Адрес: {self.__address}\n Тип: {self.__building_type}\n Этаж: {self.__floor}\n Комнат: {self.__rooms}\n Площадь: {self.__area:.2f} кв. м.")

    def to_line(self):
        """
        Вспомогательный метод для сериализации в строку для .txt.
        """
        return f"{self.__address},{self.__building_type},{self.__floor},{self.__rooms},{self.__area:.2f}"


    # Методы проверки
    def has_n_rooms(self, n):
        """Проверяет, равно ли количество комнат заданному числу."""
        return self.__rooms == n

    def is_on_floor_range(self, min_floor, max_floor):
        """Проверяет, находится ли этаж в заданном диапазоне."""
        return min_floor <= self.__floor <= max_floor

    def area_greater_than(self, min_area):
        """Проверяет, больше ли площадь заданной."""
        return self.__area > min_area

class Ms:
    """
    Класс-контейнер для списка квартир (экземпляров класса House).
    """
    def __init__(self):
        self.houses = []

    def add_house(self, house):
        """Добавляет квартиру в список."""
        self.houses.append(house)

    def input_houses_from_console(self):
        """Ввод квартир с консоли."""
        n = int(input("Сколько квартир добавить? "))
        for _ in range(n):
            h = House() # Создаём с пустыми значениями по умолчанию
            h.input_data()
            self.add_house(h)
            print("Квартира добавлена.")
        


    def load_from_file(self, filename):
        """
        Загружает список квартир из текстового файла.
        Формат: Address,Type,Floor,Rooms,Area
        """
        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        self.houses = [] # Очищаем список квартир перед чтением
        for line in lines:
            if line.strip(): # Не будет считывать пустые строки
                h = House()
                h.input_from_file(line)
                self.add_house(h)
        print(f"Данные загружены из {filename}")

    def save_to_file(self, filename):
        """
        Сохраняет список квартир в текстовый файл.
        Формат: Address,Type,Floor,Rooms,Area
        """
        with open(filename, 'w', encoding='utf-8') as f:
            for house in self.houses:
                f.write(house.to_line() + "\n")
        print(f"Данные сохранены в {filename}")

    def display_houses(self):
        """Выводит все квартиры на экран."""
        if not self.houses:
            print("Список квартир пуст.")
            return
        for i, house in enumerate(self.houses):
            print(f"--- Квартира №{i+1} ---")
            house.output()

    # Выборки данных
    def select_by_rooms(self, n):
        """Список квартир с заданным числом комнат."""
        return [h for h in self.houses if h.has_n_rooms(n)]

    def select_by_rooms_and_floor_range(self, n, min_floor, max_floor):
        """Список квартир с заданным числом комнат и в диапазоне этажей."""
        return [h for h in self.houses if h.has_n_rooms(n) and h.is_on_floor_range(min_floor, max_floor)]

    def select_by_area_greater_than(self, min_area):
        """Список квартир с площадью больше заданной."""
        return [h for h in self.houses if h.area_greater_than(min_area)]

    # Сортировка данных
    def sort_by_area(self):
        """Сортирует квартиры по площади."""
        self.houses.sort(key=lambda h: h.get_area())