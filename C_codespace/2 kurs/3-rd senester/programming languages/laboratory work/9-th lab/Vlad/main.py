from header import Shop

# Инициализируем список накопителей
container = Shop()
# Файл для записи/чтения
filename = "C:/Codespace/C_codespace/2 kurs/3-rd senester/Programming languages/laboratory work/9-th lab/Vlad/storage_devices.txt"

# Заполняем список начальными данными
with open(filename, "w", encoding='utf-8') as f:
    f.write("PortableHDD,WD Blue,1000,Western Digital,5000.0,5400\n")
    f.write("DVD,Disk A,4.7,Generic,100.0,DVD-R\n")
    f.write("PortableHDD,Seagate Fast,2000,Seagate,8000.0,7200\n")
    f.write("DVD,Disk B,4.7,Verbatim,150.0,DVD+R\n")
    f.write("PortableHDD,Toshiba Canvio,500,Toshiba,3500.0,5400\n")
    f.write("DVD,Disk C,9.4,Verbatim,200.0,DVD-RW\n")
    
while True:
    print("\n--- База данных магазина накопителей ---")
    print("1. Загрузить накопители из файла")
    print("2. Сохранить накопители в файл")
    print("3. Показать все накопители")
    print("4. Выбрать накопители по ёмкости в диапазоне")
    print("5. Показать статистику по типам накопителей")
    print("6. Отсортировать список накопителей по цене")
    print("0. Выйти")
    choice = input("Выберите действие: ")
    match choice:
        case '1':
            container.load_from_file(filename)
        case '2':
            container.save_to_file(filename)
        case '3':
            container.display_devices()
        case '4':
            min_c = int(input("Введите минимальную ёмкость: "))
            max_c = int(input("Введите максимальную ёмкость: "))
            selected = container.select_by_capacity_range(min_c, max_c)
            print(f"\n--- Накопители с ёмкостью от {min_c} до {max_c} Гб ---")
            for d in selected:
                print("----------")
                print(d.data_output())
        case '5':
            stats = container.get_stats_by_type()
            if(not stats):
                continue
            print(stats)
        case '6':
            container.sort_by_price()
            print("Список накопителей отсортирован по цене.") 
        case '0':
            print("Выход из программы...")
            break   
        case _:
            print("Неверный выбор. Пожалуйста, попробуйте снова.")