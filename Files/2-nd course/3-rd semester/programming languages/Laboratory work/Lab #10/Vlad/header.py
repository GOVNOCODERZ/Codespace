def calculate_product_and_count_above_average(numbers):
    if not numbers:
        return 1, 0  # по умолчанию: произведение ненулевых — 1; кол-во элементов выше среднего — 0

    # Произведение ненулевых элементов
    product = 1
    for x in numbers:
        if x == 0:
            continue
        product *= x

    # Среднее арифметическое
    average = sum(numbers) / len(numbers)

    # Количество элементов, превышающих среднее
    count_above_avg = sum(1 for x in numbers if x > average)

    return product, count_above_avg