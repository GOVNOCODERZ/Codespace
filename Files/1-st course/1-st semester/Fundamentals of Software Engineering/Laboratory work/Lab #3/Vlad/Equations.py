#Решение кубического уравнения через формулу Кардано
#Получаем коэффиценты степенного уравнения >> выводим корни уравнения
#Проверяем корни, подставляем в уравнение
import cmath
from cmath import *

def linear(a, b):
    solutions = set()
    if a == 0 and b == 0:
        solutions.add(True)

    if a == 0 and b != 0:
        solutions.add(False)

    if a != 0:
        solutions.add(-b / a)
    return solutions

def quadratic(massive):
    a = massive[0]
    b = massive[1]
    c = massive[2]
    solutions = []
    if a != 0:
        D = b ** 2 - 4 * a * c
        x1 = ((-b + sqrt(D)) / (2 * a))
        solutions.append(x1)
        x2 = ((-b - sqrt(D)) / (2 * a))
        solutions.append(x2)
    else:
        solutions.append(linear(b, c))
    return solutions

def cubic(massive):
    def cbrt(polynomial):
        solution = set()
        root1 = polynomial ** (1 / 3)
        root2 = root1 * (-1 / 2 + (sqrt(3) * 1j) / 2)
        root3 = root1 * (-1 / 2 - (sqrt(3) * 1j) / 2)
        solution.update({root1, root2, root3})
        return solution
    a = massive[0]
    b = massive[1]
    c = massive[2]
    d = massive[3]
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

def check(massive):
    a = massive[0]
    b = massive[1]
    c = massive[2]
    d = massive[3]
    try:
        5/a, 5/b, 5/c, 5/d
        return True
    except TypeError:
        return False
    except ValueError:
        return False
    except ZeroDivisionError:
        return False
'''
while True:
    print("Введите коэффиценты")
    a, b, c, d = map(float, input().split())
    list_coefs = [a, b, c, d]
    if check(list_coefs) == True:
        break
    else:
        print("Коэффиценты введены неверно")



answers = cubic(list_coefs)
def predel_check(massive):
    ans1 = a*massive[0]**3+b*massive[0]**2+c*massive[0]+d
    ans2 = a*massive[1]**3+b*massive[1]**2+c*massive[1]+d
    ans3 = a*massive[2]**3+b*massive[2]**2+c*massive[2]+d

    ansfloat1 = float(ans1.real)
    ansfloat2 = float(ans2.real)
    ansfloat3 = float(ans3.real)

    predel = 1.0e-9
    print(abs(ansfloat1))
    print(abs(ansfloat2), ans2)
    print(abs(ansfloat3))

    if abs(ansfloat1) >= predel:
        return False
    if abs(ansfloat2) >= predel:
        return False
    if abs(ansfloat3) >= predel:
        return False
    return True
 
#================================

def Input_koefs():
    while True:
        try:
            a, b, c, d = map(float, input("Введите коэффиценты a, b, c, d (через запятую): ").split(", "))
            break
        except ValueError:
            print("Ошибка: некорректый ввод")
        except ZeroDivisionError:
            print("Ошибка: введённые коэффиценты равны нулю")
    return a, b, c, d

def solve_cubic(a, b, c, d):
    #Находим коэффиценты p, q, а также дискриминат
    p = (3 * a * c - b**2) / (3 * a**2)
    q = (2 * b**3 - 9 * a * b * c + 27 * a**2 * d) / (27 * a**3)
    Disc = (q/2)**2 + (p/3)**3
    #Оцениваем коэффицент q, находим коэффицент f
    if q < 0:
        f = cmath.atan((cmath.sqrt(-Disc))/(-(q/2)))
    elif q > 0:
        f = cmath.atan((cmath.sqrt(-Disc))/(-(q/2))) + cmath.pi
    else:
        f = cmath.pi / 2
    #Оцениваем дискриминат, находим и возвращаем значения корней
    if Disc > 0:
        Alfa = (-q / 2 + Disc ** 0.5) ** (1 / 3)
        Beta = -abs((-q / 2 - Disc ** 0.5) ** (1 / 3))
        y1 = Alfa + Beta
        y2 = complex(-((Alfa + Beta) / 2), (Alfa - Beta) / 2 * 3 ** 0.5)
        y3 = complex(-((Alfa + Beta) / 2), -(Alfa - Beta) / 2 * 3 ** 0.5)
        x1 = y1 - b / (3 * a)
        x1 = round(x1.real, 5)
        x2 = y2 - b / (3 * a)
        x2 = round(x2.real, 5) + round(x2.imag, 5) * 1j
        x3 = y3 - b / (3 * a)
        x3 = round(x3.real, 5) + round(x3.imag, 5) * 1j                
    elif Disc < 0:
        x1 = 2 * (-p / 3)**0.5 * cmath.cos(f / 3) - b / (3 * a)
        x1 = round(x1.real, 5) 
        x2 = 2 * (-p / 3)**0.5 * cmath.cos((f / 3) + 2 * cmath.pi / 3) - b / (3 * a)
        x2 = round(x2.real, 5)
        x3 = 2 * (-p / 3)**0.5 * cmath.cos((f / 3) + 4 * cmath.pi / 3) - b / (3 * a)
        x3 = round(x3.real, 5)
    else:
        x1 = 2 * (-q / 2) ** (1 / 3) - b / (3 * a)
        x2 = (-q / 2) ** (-1 / 3) - b / (3 * a)
        x3 = (-q / 2) ** (-1 / 3) - b / (3 * a)
    return x1, x2, x3
'''