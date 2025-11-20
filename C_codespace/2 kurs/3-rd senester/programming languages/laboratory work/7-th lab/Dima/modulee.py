def read_numbers(filename):
    try:
        with open(filename, 'r') as f:
            line = f.read()
            numbers = [int(n) for n in line.split()]
            return numbers
    except FileNotFoundError:
        print(f"Ошибка: Файл '{filename}' не найден.")
        return []

def find_zeros(numbers):
    if not numbers:
        return 0

    max_count = 0
    current_count = 0

    for num in numbers:
        if num == 0:
            current_count += 1
        else:
            if current_count > max_count:
                max_count = current_count
            current_count = 0
    
    if current_count > max_count:
        max_count = current_count
        
    return max_count

def write_result(result, filename):
    try:
        with open(filename, 'w') as f:
            f.write(f"Наибольшее количество подряд идущих нулей: {result}")
        print(f"Результат записан в файл '{filename}'.")
    except IOError:
        print(f"Ошибка записи в файл '{filename}'.")