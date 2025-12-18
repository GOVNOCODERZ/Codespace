from modulee import Tour, TourManager, process_search_result
def main():
    manager = TourManager()
    
    while True:
        print("\n" + "="*30)
        print(" МЕНЮ ТУРАГЕНТСТВА ")
        print("="*30)
        print("1. Загрузить данные из файла")
        print("2. Добавить туры вручную")
        print("3. Показать весь список")
        print("4. Сортировать весь список по длительности")
        print("5. Сохранить весь список в файл")
        print("-" * 20)
        print("6. Найти: по длительности (> дней)")
        print("7. Найти: Агентство + Длительность")
        print("8. Найти: Страна + Цена (от и до)")
        print("-" * 20)
        print("0. Выход")
        
        choice = input("\nКоманда: ")

        if choice == "1":
            fname = input("Имя файла (Enter = tours.csv): ") or "tours.csv"
            manager.load_file(fname)

        elif choice == "2":
            try:
                count = int(input("Сколько туров добавить? "))
                manager.fill_keyboard(count)
            except ValueError:
                print("Нужно ввести число.")

        elif choice == "3":
            manager.display()

        elif choice == "4":
            manager.sort_duration()

        elif choice == "5":
            fname = input("Имя файла (Enter = saved.csv): ") or "saved.csv"
            manager.save_file(fname)

        elif choice == "6":
            try:
                days = int(input("Длительность больше (дней): "))
                found = manager.filter_duration(days)
                process_search_result(found)
            except ValueError:
                print("Ошибка: введите число.")

        elif choice == "7":
            try:
                agency = input("Агентство: ")
                days = int(input("Длительность больше (дней): "))
                found = manager.filter_agency_duration(agency, days)
                process_search_result(found)
            except ValueError:
                print("Ошибка ввода.")

        elif choice == "8":
            try:
                country = input("Страна: ")
                min_p = float(input("Цена от: "))
                max_p = float(input("Цена до: "))
                found = manager.filter_country_price(country, min_p, max_p)
                process_search_result(found)
            except ValueError:
                print("Ошибка ввода чисел.")

        elif choice == "0":
            print("Выход.")
            break
        else:
            print("Неизвестная команда.")

if __name__ == "__main__":
    main()