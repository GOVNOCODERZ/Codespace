from random import *
import cmath
from cmath import*

proverka = []

for i in range(100):
    #Рандомный выбор значения для коэффицента с исключением нуля
    a = choice((randint(-10,-1),randint(1,11)))
    b = choice((randint(-10,-1),randint(1,11)))
    c = choice((randint(-10,-1),randint(1,11)))
    d = choice((randint(-10,-1),randint(1,11)))

    def cbrt(polynomial):
        solution = set()
        root1 = polynomial ** (1 / 3)
        root2 = (polynomial ** (1 / 3)) * (-1 / 2 + (sqrt(3) * 1j) / 2)
        root3 = (polynomial ** (1 / 3)) * (-1 / 2 - (sqrt(3) * 1j) / 2)
        solution.update({root1, root2, root3})
        return solution


    def linear(a, b):
        solutions = set()
        if a == 0 and b == 0:
            solutions.add(True)

        if a == 0 and b != 0:
            solutions.add(False)

        if a != 0:
            solutions.add(-b / a)
        return solutions


    def quadratic(a, b, c):
        solutions = set()
        if a != 0:
            D = b ** 2 - 4 * a * c
            x1 = (-b + sqrt(D)) / (2 * a)
            x2 = (-b - sqrt(D)) / (2 * a)
            solutions.update({x1, x2})
        else:
            solutions.update(linear(b, c))
        return solutions


    def cubic(a, b, c, d):
        solutions = []
        if a != 0:
            p = (3 * a * c - b ** 2) / (3 * a ** 2)
            q = (2 * b ** 3 - 9 * a * b * c + 27 * a ** 2 * d) / (27 * a ** 3)
            alpha = cbrt(-q / 2 + sqrt((q / 2) ** 2 + (p / 3) ** 3))
            beta = cbrt(-q / 2 - sqrt((q / 2) ** 2 + (p / 3) ** 3))
            for i in alpha:
                for j in beta:
                    if abs((i * j) + p / 3) <= 0.0001:
                        x = i + j - b / (3 * a)
                        solutions.append(x)
        else:
            solutions.update(quadratic(b, c, d))
        return solutions

    amass = cubic(a,b,c,d)
    #Подстановка корней в уравнение (ax^3+bx^2+cx+d = 0)
    ans1 = a*amass[0]**3+b*amass[0]**2+c*amass[0]+d
    ans2 = a*amass[1]**3+b*amass[1]**2+c*amass[1]+d
    ans3 = a*amass[2]**3+b*amass[2]**2+c*amass[2]+d
    #Перевод из complex в float
    ansfloat1 = float(ans1.real)
    ansfloat2 = float(ans2.real)
    ansfloat3= float(ans3.real)
    #Занесение значений, полученных после подстановки, в массив
    proverka.append(abs(ansfloat1))
    proverka.append(abs(ansfloat2))
    proverka.append(abs(ansfloat3))
#Вывод максимального числа массива (для нахождения границы погрешности)
print(max(proverka))
    
# Минимальное число - 1.0e-10
# Брать границу величиной в 1.0e-9