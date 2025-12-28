from vehicles import TransportCompany, Train, Airplane

def handle_search_result(manager):
    """Вспомогательная функция для работы с результатами поиска."""
    if manager.is_empty():
        print("По вашему запросу ничего не найдено.")
        return

    print(f"\nНайдено записей: {manager.get_count()}")
    while True:
        print("\n[Действия с результатом]")
        print("1. Показать на экране")
        print("2. Сохранить в файл")
        print("0. Вернуться назад")
        choice = input("Выбор: ")
        
        if choice == "1":
            manager.display()
        elif choice == "2":
            fname = input("Имя файла для сохранения: ")
            manager.save_file(fname)
        elif choice == "0":
            break
        else:
            print("Неверный ввод.")

def main():
    company = TransportCompany()
    
    while True:
        print("\n=== ТРАНСПОРТНАЯ КОМПАНИЯ ===")
        print("1. Добавить Поезд")
        print("2. Добавить Самолет")
        print("3. Показать весь транспорт")
        print("4. Сохранить в файл")
        print("5. Загрузить из файла")
        print("--- Обработка данных ---")
        print("6. Найти транспорт со сроком доставки МЕНЬШЕ Х")
        print("7. Найти транспорт с грузоподъемностью БОЛЬШЕ Х")
        print("8. Сортировать по возрастанию объема")
        print("0. Выход")
        
        choice = input("Ваш выбор: ")

        if choice == "1":
            company.add(Train.from_keyboard())
        elif choice == "2":
            company.add(Airplane.from_keyboard())
        elif choice == "3":
            company.display()
        elif choice == "4":
            fname = input("Имя файла [db.csv]: ") or "db.csv"
            company.save_file(fname)
        elif choice == "5":
            fname = input("Имя файла [db.csv]: ") or "db.csv"
            company.load_file(fname)
        elif choice == "6":
            try:
                val = int(input("Введите предельный срок (ч): "))
                res = company.filter_time_below(val)
                handle_search_result(res)
            except ValueError: print("Ошибка: ожидалось целое число.")
        elif choice == "7":
            try:
                val = float(input("Введите мин. грузоподъемность (т): "))
                res = company.filter_capacity_above(val)
                handle_search_result(res)
            except ValueError: print("Ошибка: ожидалось число.")
        elif choice == "8":
            company.sort_by_volume()
            company.display()
        elif choice == "0":
            break
        else:
            print("Неверный пункт меню.")

if __name__ == "__main__":
    main()