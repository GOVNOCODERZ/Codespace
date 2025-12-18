from modulee import TransportCompany, process_search_result, Train, Airplane
def main():
    company = TransportCompany()

    while True:
        print("\n" + "="*40)
        print(" ТРАНСПОРТНАЯ КОМПАНИЯ ")
        print("="*40)
        print("1. Добавить транспорт вручную")
        print("2. Загрузить из файла")
        print("3. Показать весь список")
        print("4. Сохранить весь список в файл")
        print("-" * 30)
        print("5. Найти: Срок доставки МЕНЬШЕ заданного")
        print("6. Найти: Грузоподъемность БОЛЬШЕ заданной")
        print("7. Сортировать по ВОЗРАСТАНИЮ объема")
        print("-" * 30)
        print("0. Выход")

        choice = input("\nВаш выбор: ")

        if choice == "1":
            print("\nТип транспорта:")
            print("1. Поезд (Train)")
            print("2. Самолет (Airplane)")
            t_choice = input("Выбор: ")
            if t_choice == "1":
                company.add(Train.from_keyboard())
            elif t_choice == "2":
                company.add(Airplane.from_keyboard())
            else:
                print("Неверный тип.")

        elif choice == "2":
            fname = input("Имя файла (Enter для 'transport.csv'): ") or "transport.csv"
            company.load_file(fname)

        elif choice == "3":
            company.display()

        elif choice == "4":
            fname = input("Имя файла (Enter для 'transport_saved.csv'): ") or "transport_saved.csv"
            company.save_file(fname)

        elif choice == "5":
            try:
                val = int(input("Введите максимальный срок доставки (часов): "))
                found = company.filter_time_below(val)
                process_search_result(found)
            except ValueError:
                print("Введите число.")

        elif choice == "6":
            try:
                val = float(input("Введите минимальную грузоподъемность (тонн): "))
                found = company.filter_capacity_above(val)
                process_search_result(found)
            except ValueError:
                print("Введите число.")

        elif choice == "7":
            company.sort_by_volume()
            company.display()

        elif choice == "0":
            print("Завершение работы.")
            break
        else:
            print("Неверная команда.")

if __name__ == "__main__":
    main()