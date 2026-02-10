# main.py

# Подключаем наш модуль с функциями
import modulee

def main():
    input_filename = 'input.txt'
    output_filename = 'output.txt'

    # Шаг 1: Читаем числа из файла
    numbers = modulee.read_numbers(input_filename)
    print(f"Прочитали список: {numbers}")

    # Если список пустой, выходим
    if not numbers:
        return

    # Шаг 2: Находим ответ
    result = modulee.find_zeros(numbers)
    print(f"Результат: {result}")

    # Шаг 3: Записываем ответ в другой файл
    modulee.write_result(result, output_filename)

# Этот блок выполняется при запуске файла
if __name__ == "__main__":
    # Создаем тестовый файл input.txt для проверки
    with open('input.txt', 'w') as f:
        f.write("1 0 5 0 0 8 0 0 0 0 2 0 0 0 9")
    main()