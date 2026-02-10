from cmath import *

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


def cubic(x):
    a = x[0]
    b = x[1]
    c = x[2]
    d = x[3]
    print(a,b,c,d)
    if a == 0 or b == 0 or c == 0 or d == 0:
        raise ValueError('Аргументы должны быть не равны нулю')
    if type(a) not in [int,float] or type(b) not in [int,float] or type(c) not in [int,float] or type(d) not in [int,float] :
        raise TypeError('Аргументы должны быть либо целыми, либо дробными числами')

    
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


# def check():
#     while True:
#         try:
#             print("Введите коэффиценты")
#             a, b, c, d = map(float, input().split())
#             5/a, 5/b, 5/c, 5/d
#             break
#         except ValueError:
#             print("Коэффиценты введены неверно")
#         except ZeroDivisionError:
#             print("Коэффиценты не могут быть равны нулю")
#     return a, b, c, d


# a, b, c, d = check()

# print('Корни уравнения: ',cubic(a, b, c, d))
# amass = cubic(a,b,c,d)
# #Подстановка корней в уравнение (ax^3+bx^2+cx+d = 0)
# ans1 = a*amass[0]**3+b*amass[0]**2+c*amass[0]+d
# ans2 = a*amass[1]**3+b*amass[1]**2+c*amass[1]+d
# ans3 = a*amass[2]**3+b*amass[2]**2+c*amass[2]+d
# #Перевод из complex в float
# ansfloat1 = float(ans1.real)
# ansfloat2 = float(ans2.real)
# ansfloat3 = float(ans3.real)
# print('Значения уравнения с подставленными корнями: ',ans1,ans2,ans3)
# #Проверка подстановленных корней в уравнение. Граница погрешности найдена в файле "Cube test minmax error"
# if abs(ansfloat1) < 1.0e-9:
#     print("Первый корень прошёл проверку при максимальной границе в 1.0e-9")
# else:
#     print("Корень не прошёл проверку. Возможно, произошла ошибка вычисления, либо корень подходит, но больше границы погрешности")
# if abs(ansfloat2) < 1.0e-9:
#     print("Второй корень прошёл проверку при максимальной границе в 1.0e-9")
# else:
#     print("Корень не прошёл проверку. Возможно, произошла ошибка вычисления, либо корень подходит, но больше границы погрешности")
# if abs(ansfloat3) < 1.0e-9:
#     print("Третий корень прошёл проверку при максимальной границе в 1.0e-9")
# else:
#     print("Корень не прошёл проверку. Возможно, произошла ошибка вычисления, либо корень подходит, но больше границы погрешности")