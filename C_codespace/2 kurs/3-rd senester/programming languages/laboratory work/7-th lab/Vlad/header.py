def read_data_from_file(file_path):
    try:
        with open(file_path, 'r') as file:
            content = file.read() # Читаем содержимое файла
            numbers_str = content.split() # Разбиваем в список строк по пробелам
            numbers = [int(num_str) for num_str in numbers_str] # Преобразуем строки в числа
        return numbers
    except FileNotFoundError:
        print(f"ОШИБКА: Файл {file_path} не найден.")
        return
    except ValueError:
        print(f"ОШИБКА: Невозможно преобразовать данные в целые числа в файле {file_path}.")
        return


def process_data(data_list):
    if not data_list:
        print("ОШИБКА: Список пуст.")
        return

    min_val = min(data_list) # Находим минимальный элемент среди всех
    
    even_numbers = [num for num in data_list if num % 2 == 0] # Список чётных элементов

    if not even_numbers:
        print("В списке нет чётных чисел.")
        max_even = None
    else:
        max_even = max(even_numbers)

    return max_even, min_val


def write_result_to_file(result, file_path):
    max_even, min_all = result
    try:
        with open(file_path, 'w', encoding='utf-8') as file:
            if max_even is None:
                file.write("Максимальный чётный элемент: не найден\n")
            else:
                file.write(f"Максимальный чётный элемент: {max_even}\n")
            if min_all is None:
                file.write("Минимальный элемент: не найден\n")
            else:
                file.write(f"Минимальный элемент: {min_all}\n")

        print(f"Результаты успешно записаны в {file_path}")
    except Exception as e:
        print(f"ОШИБКА при записи в файл {file_path}: {e}")