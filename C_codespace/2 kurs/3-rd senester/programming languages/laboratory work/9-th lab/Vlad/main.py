from media_module import Ms # Импортируем класс-контейнер из модуля

def main():
    container = Ms()
    filename = "storage_devices.txt"

    while True:
        print("\n--- База данных магазина накопителей ---")
        print("1. Добавить накопители с консоли")
        print("2. Загрузить накопители из файла")
        print("3. Сохранить накопители в файл")
        print("4. Показать все накопители")
        print("5. Выбрать накопители по ёмкости (диапазон)")
        print("6. Выбрать накопители по типу")
        print("7. Показать статистику по типам")
        print("8. Отсортировать накопители по ёмкости")
        print("0. Выйти")
        choice = input("Выберите действие: ")

        if choice == '1':
            container.input_devices_from_console()
        elif choice == '2':
            container.load_from_file(filename)
        elif choice == '3':
            container.save_to_file(filename)
        elif choice == '4':
            container.display_devices()
        elif choice == '5':
            try:
                min_c = int(input("Введите минимальную ёмкость: "))
                max_c = int(input("Введите максимальную ёмкость: "))
                selected = container.select_by_capacity_range(min_c, max_c)
                print(f"\n--- Накопители с ёмкостью от {min_c} до {max_c} Гб ---")
                for d in selected:
                    d.output()
            except ValueError:
                print("Неверный ввод для ёмкости.")
        elif choice == '6':
            device_type = input("Введите тип накопителя (HDD или FlashD): ").strip()
            if device_type not in ["HDD", "FlashD"]:
                print("Неверный тип. Введите HDD или FlashD.")
                continue
            selected = container.select_by_type(device_type)
            print(f"\n--- Накопители типа {device_type} ---")
            for d in selected:
                d.output()
        elif choice == '7':
            stats = container.get_stats_by_type()
            print(stats)
        elif choice == '8':
            container.sort_by_capacity()
            print("Накопители отсортированы по ёмкости.")
        elif choice == '0':
            print("Выход...")
            break
        else:
            print("Неверный выбор. Пожалуйста, попробуйте снова.")

if __name__ == "__main__":
    main()