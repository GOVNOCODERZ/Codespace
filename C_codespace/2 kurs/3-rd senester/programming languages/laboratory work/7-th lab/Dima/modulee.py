# my_module.py

def read_numbers(filename):
    """Читает числа из файла и возвращает их как список."""
    try:
        # Открываем файл для чтения
        with open(filename, 'r') as f:
            line = f.read()
            # Превращаем строку в список чисел
            numbers = [int(n) for n in line.split()]
            return numbers
    except FileNotFoundError:
        print(f"Ошибка: Файл '{filename}' не найден.")
        return [] # Возвращаем пустой список, если файла нет

def find_zeros(numbers):
    """Находит максимальное количество нулей подряд."""
    if not numbers:
        return 0

    max_count = 0
    current_count = 0

    # Проходим по каждому числу в списке
    for num in numbers:
        if num == 0:
            # Если число - ноль, увеличиваем счетчик
            current_count += 1
        else:
            # Если число не ноль, проверяем, не был ли счетчик максимальным
            if current_count > max_count:
                max_count = current_count
            # Сбрасываем счетчик
            current_count = 0
    
    # Последняя проверка после цикла (вдруг нули были в конце)
    if current_count > max_count:
        max_count = current_count
        
    return max_count

def write_result(result, filename):
    """Записывает результат в файл."""
    try:
        # Открываем файл для записи
        with open(filename, 'w') as f:
            f.write(f"Наибольшее количество подряд идущих нулей: {result}")
        print(f"Результат записан в файл '{filename}'.")
    except IOError:
        print(f"Ошибка записи в файл '{filename}'.")