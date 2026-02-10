import time
import sys
import numpy as np
from random import *
from math import *

sys.set_int_max_str_digits(1000000000)

class Functions():
    def get_data():
        size = int(input("Укажите размер массива: "))
        left_border = float(input("Укажите наименьшее допустимое число в массиве: "))
        right_border = float(input("Укажите наибольшее допустимое число в массиве: "))
        
        return size, left_border, right_border

    def generation_int(size, left_border, right_border):
        global gen_time, massive, operation_type, gen_range
        
        left_border = int(left_border)
        right_border = int(right_border)
        gen_range = f"[{left_border}, {right_border}]"
        time_start = time.time()
        massive = np.random.randint(left_border, right_border + 1, size) # в randint() верхняя граница по умолчанию не включается
        time_end = time.time()
        gen_time = float(time_end - time_start)
        
    def generation_float(size, left_border, right_border):
        global gen_time, massive, operation_type, gen_range
        
        left_border = float(left_border)
        right_border = float(right_border)
        gen_range = f"[{left_border}, {right_border}]"
        time_start = time.time()
        massive = np.random.uniform(left_border, right_border, size)
        time_end = time.time()
        gen_time = float(time_end - time_start)
    
class Operations():
    def addition():
        global operation_time, value_total, massive
        
        time_start = time.time()
        for i in massive:
            value_total += i
        time_end = time.time()
        operation_time = float(time_end - time_start)
    
    def subtraction():
        global operation_time, value_total, massive
        
        time_start = time.time()
        for i in massive:
            value_total -= i
        time_end = time.time()
        operation_time = float(time_end - time_start)

    def multiplication():
        global operation_time, value_total, massive
        
        value_total = 1
        time_start = time.time()
        for i in massive:
            value_total *= i
        time_end = time.time()
        operation_time = float(time_end - time_start)
    
    def division():
        global operation_time, value_total, massive
        
        value_total = 1
        time_start = time.time()
        for i in massive:
            value_total /= i
        time_end = time.time()
        operation_time = float(time_end - time_start)
    
    def square_root():
        global operation_time, value_total, massive
        
        time_start = time.time()
        value_total = "X"
        for i in range(0, len(massive)-1):
            massive[i] ** 0.5
        time_end = time.time()
        operation_time = float(time_end - time_start)
    
    def square():
        global operation_time, value_total, massive
        
        time_start = time.time()
        value_total = "X"
        for i in range(0, len(massive)-1):
            massive[i] ** 2
        time_end = time.time()
        operation_time = float(time_end - time_start)
        
while True:
    operation_type = ''
    data_type = ''
    operation_time = 0
    gen_range = ''
    gen_time = 0
    list_size = 0
    value_total = 0
    massive = []
    opt1 = input("Выберите опцию: \n1 - Начать работу\n2 - Выход\nВвод: ")
    match opt1.split():
        case ["1"]:
            list_size, left_border, right_border = Functions.get_data()
            opt2 = input("Выберите операцию: \n1 - Сложение\n2 - Вычитание\n3 - Умножение\n4 - Деление\n5 - Квадратный корень\n6 - Возведение во вторую степень\nВвод: ")
            match opt2.split():
                case ["1"]:
                    operation_type = "Сложение"
                    data_type = input("Выберите тип данных: \n1 - Целые числа (int)\n2 - Числа с плавающей точкой (float)\nВвод: ")
                    match data_type.split():
                        case ["1"]:
                            Functions.generation_int(list_size, left_border, right_border)
                            data_type = "int"
                        case ["2"]:
                            Functions.generation_float(list_size, left_border, right_border)
                            data_type = "float"
                    Operations.addition()
                case ["2"]:
                    operation_type = "Вычитание"
                    data_type = input("Выберите тип данных: \n1 - Целые числа (int)\n2 - Числа с плавающей точкой (float)\nВвод: ")
                    match data_type.split():
                        case ["1"]:
                            Functions.generation_int(list_size, left_border, right_border)
                            data_type = "int"
                        case ["2"]:
                            Functions.generation_float(list_size, left_border, right_border)
                            data_type = "float"
                    Operations.subtraction()
                case ["3"]:
                    operation_type = "Умножение"
                    print("\nНеизменяемый тип данных для умножения - число с плавающей точкой (float)")
                    Functions.generation_float(list_size, left_border, right_border)
                    data_type = "float"
                    Operations.multiplication()                    
                case ["4"]:
                    operation_type = "Деление"
                    print("\nНеизменяемый тип данных для деления - число с плавающей точкой (float)")
                    Functions.generation_float(list_size, left_border, right_border)
                    data_type = "float"
                    Operations.division()
                case ["5"]:
                    operation_type = "Квадратный корень числа"
                    data_type = input("Выберите тип данных: \n1 - Целые числа (int)\n2 - Числа с плавающей точкой (float)\nВвод: ")
                    match data_type.split():
                        case ["1"]:
                            Functions.generation_int(list_size, left_border, right_border)
                            data_type = "int"
                        case ["2"]:
                            Functions.generation_float(list_size, left_border, right_border)
                            data_type = "float"
                    Operations.square_root()
                case ["6"]:
                    operation_type = "Квадрат числа"
                    data_type = input("Выберите тип данных: \n1 - Целые числа (int)\n2 - Числа с плавающей точкой (float)\nВвод: ")
                    match data_type.split():
                        case ["1"]:
                            Functions.generation_int(list_size, left_border, right_border)
                            data_type = "int"
                        case ["2"]:
                            Functions.generation_float(list_size, left_border, right_border)
                            data_type = "float"     
                    Operations.square()
            print(f"\nПервые 10 элементов массива: {massive[0:9]}\nМаксимальный элемент - {max(massive)}\nМинимальный элемент - {min(massive)}")
            print(f"\nТип операции - {operation_type}\nТип данных - {data_type}\nРазмер массива - {list_size}\nДиапазон - {gen_range}\nВремя генерации - {gen_time}\nВремя проведения операции - {operation_time}\nПолученное значение - {value_total}\n")                                                    
        case ["2"]:
            print("\nПрограмма закрыта")
            break