# main.py
from modulee import TransportCompany

# Функция для отображения меню.
def print_menu():
    print("\n--- Меню ---")
    print("1. Загрузить данные из файла")
    print("2. Показать весь транспорт")
    print("3. Показать транспорт с доставкой быстрее указанного срока (часы)")
    print("4. Показать транспорт с грузоподъемностью выше указанной (кг)")
    print("5. Показать транспорт, отсортированный по объему груза")
    print("0. Выход")
    print("------------")

# Основная функция программы.
def main():
    # Создаем экземпляр компании.
    company = TransportCompany()
    
    # Создадим демонстрационный файл с данными.
    try:
        with open("vehicles.txt", "w", encoding='utf-8') as f:
            f.write("Train, Товарный-105, 500000, 1200, 72, 60\n")
            f.write("Airplane, Ан-124 Руслан, 120000, 750, 12, 8000\n")
            f.write("Train, Сапсан-груз, 150000, 400, 24, 10\n")
            f.write("Airplane, Boeing 747-8F, 134000, 855, 15, 14000\n")
            f.write("Train, Контейнерный-5, 450000, 1000, 96, 55\n")
    except Exception as e:
        print(f"Не удалось создать файл с данными: {e}")

    # Основной цикл программы.
    while True:
        print_menu()
        choice = input("Выберите пункт меню: ")

        if choice == '1':
            filename = input("Введите имя файла для загрузки (по умолч. 'vehicles.txt'): ")
            if not filename:
                filename = "vehicles.txt"
            company.load_from_file(filename)
        elif choice == '2':
            company.show_all()
        elif choice == '3':
            try:
                max_time = int(input("Введите максимальное время доставки: "))
                company.show_by_delivery_time(max_time)
            except ValueError:
                print("Ошибка: введите целое число.")
        elif choice == '4':
            try:
                min_capacity = int(input("Введите минимальную грузоподъемность: "))
                company.show_by_capacity(min_capacity)
            except ValueError:
                print("Ошибка: введите целое число.")
        elif choice == '5':
            company.show_sorted_by_volume()
        elif choice == '0':
            print("Завершение работы программы.")
            break
        else:
            print("Неверный пункт меню, попробуйте снова.")

# Точка входа в программу.
if __name__ == "__main__":
    main()