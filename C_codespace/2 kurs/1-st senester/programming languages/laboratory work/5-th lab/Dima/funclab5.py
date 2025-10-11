def inp():
    A_input = input('Введите вещественные числа через пробел: ')
    A = list(map(float, A_input.split())) #Ввод вещественных чисел
    return(A)

def massBForm(mass):
    B = []
    cacheB = [i for i in mass if i>=0] #реализация через списоквое включение
    # for i in mass:
    #     if i >= 0:
    #         cnt += 1 
    #         summpol += i
    B.append(max(mass))    #Заполнение массива
    B.append(sum(cacheB)/len(cacheB))
    B.append(abs(min(mass)))
    return(B)