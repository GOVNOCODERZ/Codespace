from header import Ms

def main():
    container = Ms()
    filename = "C_codespace/2 kurs/3-rd senester/Programming languages/laboratory work/8-th lab/Vlad/apartments.txt"

    while True:
        print("\n--- Система управления квартирами ---")
        print("1. Добавить квартиры с консоли")
        print("2. Загрузить квартиры из файла")
        print("3. Сохранить квартиры в файл")
        print("4. Показать все квартиры")
        print("5. Выбрать квартиры по количеству комнат")
        print("6. Выбрать квартиры по комнатам и диапазону этажей")
        print("7. Выбрать квартиры по площади больше заданной")
        print("8. Отсортировать квартиры по площади")
        print("0. Выйти")
        choice = input("Выберите действие: ")

        if choice == '1':
            container.input_houses_from_console()
        elif choice == '2':
            container.load_from_file(filename)
        elif choice == '3':
            container.save_to_file(filename)
        elif choice == '4':
            container.display_houses()
        elif choice == '5':
            try:
                n = int(input("Введите количество комнат: "))
                selected = container.select_by_rooms(n)
                print(f"\n--- Квартиры с {n} комнатами ---")
                for h in selected:
                    h.output()
            except ValueError:
                print("Неверный ввод для количества комнат.")
        elif choice == '6':
            try:
                n = int(input("Введите количество комнат: "))
                min_f = int(input("Введите минимальный этаж: "))
                max_f = int(input("Введите максимальный этаж: "))
                selected = container.select_by_rooms_and_floor_range(n, min_f, max_f)
                print(f"\n--- Квартиры с {n} комнатами на этажах {min_f}-{max_f} ---")
                for h in selected:
                    h.output()
            except ValueError:
                print("Неверный ввод для этажей или количества комнат.")
        elif choice == '7':
            try:
                min_a = float(input("Введите минимальную площадь: "))
                selected = container.select_by_area_greater_than(min_a)
                print(f"\n--- Квартиры с площадью > {min_a} кв. м. ---")
                for h in selected:
                    h.output()
            except ValueError:
                print("Неверный ввод для площади.")
        elif choice == '8':
            container.sort_by_area()
            print("Квартиры отсортированы по площади.")
        elif choice == '0':
            print("Выход...")
            break
        else:
            print("Неверный выбор. Пожалуйста, попробуйте снова.")

if __name__ == "__main__":
    main()