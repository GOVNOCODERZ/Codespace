import header

# Пути к файлам
input_file = "D:/Codespace/C_codespace/2 kurs/3-rd senester/Programming languages/laboratory work/7-th lab/Vlad/input.txt"
output_file = "D:/Codespace/C_codespace/2 kurs/3-rd senester/Programming languages/laboratory work/7-th lab/Vlad/output.txt"

print(f"Чтение данных из {input_file}...")
data = header.read_data_from_file(input_file)

if data:
    print(f"\nДанные из файла: {data}")

    print("\nОбработка данных...")
    result = header.process_data(data)

    print("\nЗапись результата в файл...")
    header.write_result_to_file(result, output_file)

    max_even, min_all = result
    print("\n=== Результаты ===")
    if max_even:
        print(f"Максимальный чётный элемент: {max_even}")
    else:
        print("Максимальный чётный элемент: не найден")
    if min_all:
        print(f"Минимальный элемент: {min_all}")
    else:
        print("Минимальный элемент: не найден")
else:
    print("ОШИБКА: Не удалось прочитать данные из файла или файл пуст.")