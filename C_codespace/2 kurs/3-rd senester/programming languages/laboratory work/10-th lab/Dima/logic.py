def process_data(numbers):
    # Отбираются только положительные числа
    positive_numbers = [num for num in numbers if num > 0]

    if not positive_numbers:
        raise ValueError("В списке нет положительных элементов.")

    min_positive = min(positive_numbers)
    count = positive_numbers.count(min_positive)

    return min_positive, count