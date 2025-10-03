from list_operations import fill_list_from_input, process_list

def main():
    # 1. заполняем список A с клавиатуры
    list_A = fill_list_from_input()

    # 2. обрабатываем список A для получения списка B
    list_B = process_list(list_A)

    # 3. выводим результаты
    print("\nРезультаты")
    print(f"Исходный список A: {list_A}")
    print(f"Сформированный список B: {list_B}")
    print(f"  - B[0] (Максимум в A): {list_B[0]}")
    print(f"  - B[1] (Среднее арифметическое положительных в A): {list_B[1]:.2f}")
    print(f"  - B[2] (Абсолютное значение минимума в A): {list_B[2]}")

if __name__ == "__main__":
    main()