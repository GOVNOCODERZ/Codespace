# logic.py

def process_data(numbers):
    """
    Находит минимальный положительный элемент в списке и их количество.

    Args:
        numbers: Список чисел.

    Returns:
        Кортеж (минимальный_положительный_элемент, количество).
    """
    # Отбираем только положительные числа
    positive_numbers = [num for num in numbers if num > 0]

    # Если положительных чисел нет, вызываем исключение
    if not positive_numbers:
        raise ValueError("В списке нет положительных элементов.")

    # Находим минимальное среди положительных
    min_positive = min(positive_numbers)
    # Считаем, сколько раз оно встречается
    count = positive_numbers.count(min_positive)

    return min_positive, count