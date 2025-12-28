import os
import csv
from tours import TourManager

def handle_search_result(manager_result):
    """Обработка и вывод результатов поиска."""
    if manager_result.is_empty():
        print("По вашему запросу ничего не найдено.")
        return

    print(f"\nНайдено записей: {manager_result.get_count()}")
    manager_result.display()
    
    if input("Сохранить результат в файл? (да/нет): ").lower() == 'да':
        fname = input("Введите имя файла: ")
        manager_result.save_to_file(fname)

def main():
    manager = TourManager()
    
    while True:
        print("\n=== МЕНЮ УПРАВЛЕНИЯ ТУРАМИ ===")
        print("1. Загрузить из файла (db.csv)")
        print("2. Добавить вручную")
        print("3. Показать все туры")
        print("4. Сохранить все туры в файл")
        print("5. Сортировать по длительности")
        print("--- Поиск и выборка ---")
        print("6. Найти туры длительностью больше Х дней")
        print("7. Найти туры конкретного агентства (длительность > Х)")
        print("8. Найти туры в страну с ценой в диапазоне")
        print("0. Выход")
        
        choice = input("Ваш выбор: ")

        if choice == "1":
            fname = input("Имя файла [db.csv]: ") or "db.csv"
            manager.load_from_file(fname)
        elif choice == "2":
            manager.fill_from_keyboard()
        elif choice == "3":
            manager.display()
        elif choice == "4":
            fname = input("Имя файла [out.csv]: ") or "out.csv"
            manager.save_to_file(fname)
        elif choice == "5":
            manager.sort_by_duration()
            manager.display()
        elif choice == "6":
            try:
                d = int(input("Минимальное кол-во дней: "))
                res = manager.find_long_tours(d)
                handle_search_result(res)
            except ValueError:
                print("Ошибка ввода числа.")
        elif choice == "7":
            try:
                ag = input("Агентство: ")
                d = int(input("Минимальное кол-во дней: "))
                res = manager.find_by_agency_duration(ag, d)
                handle_search_result(res)
            except ValueError: 
                print("Ошибка ввода числа.")
        elif choice == "8":
            try:
                cnt = input("Страна: ")
                min_p = float(input("Цена от: "))
                max_p = float(input("Цена до: "))
                res = manager.find_by_country_price(cnt, min_p, max_p)
                handle_search_result(res)
            except ValueError: 
                print("Ошибка ввода числа.")
        elif choice == "0":
            break
        else:
            print("Неверный пункт меню.")

if __name__ == "__main__":
    # Генерация тестового файла, если он отсутствует
    if not os.path.exists("db.csv"):
        with open("db.csv", "w", encoding="utf-8", newline="") as f:
            w = csv.writer(f)
            w.writerows([
                ["Tui", "Пляжный отдых", "Турция", 7, 50000],
                ["Pegas", "Тайская сказка", "Тайланд", 14, 120000],
                ["Tui", "Евротур", "Италия", 10, 95000],
                ["Anex", "Красное море", "Египет", 6, 45000],
            ])
    
    main()