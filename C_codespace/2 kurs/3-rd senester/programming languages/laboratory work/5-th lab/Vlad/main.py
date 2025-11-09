import header

print("=== Ввод списка A ===")
list_A = header.list_input()
print("=== Ввод списка B ===")
list_B = header.list_input()

second_max = header.find_second_max_index(list_A)
penultimate_min = header.find_penultimate_min_index(list_B)
elem_sum = list_A[second_max] + list_B[penultimate_min]

print(f"Индекс второго максимума списка A: {second_max}")
print(f"Индекс предпоследнего минимума списка B: {penultimate_min}")
print(f"Сумма этих значений: {elem_sum}")