def list_input():
    A_input = input("Введите целые числа через пробел: ")
    A = list(map(int, A_input.split()))
    return(A)

def find_second_max_index(list):
    if len(list) < 2: # Слишком мало элементов
        return

    unique_sorted = sorted(set(list), reverse=True) # Список без дубликатов, сортировка по убыванию

    if len(unique_sorted) < 2: # Все элементы одинаковы
        return

    second_max_val = unique_sorted[1] # Второй элемент списка = второй максимум

    for i, val in enumerate(list):
        if val == second_max_val:
            return i # Индекс второго максимума
        
def find_penultimate_min_index(list):
    if len(list) < 2: # Слишком мало элементов
        return

    unique_sorted = sorted(set(list)) # Список без дубликатов, сортировка по возрастанию

    if len(unique_sorted) < 2: # Все элементы одинаковы
        return

    second_max_val = unique_sorted[len(unique_sorted) - 2] # Предпоследний элемент списка = второй минимум

    for i, val in enumerate(list):
        if val == second_max_val:
            return i # Индекс предпоследнего минимума