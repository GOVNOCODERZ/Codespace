from prettytable import PrettyTable
class Car():
    def __init__(self, number, brand, model, engine_type, color, power):
        self.number = number
        self.brand = brand
        self.model = model
        self.engine_type = engine_type
        self.color = color
        self.power = power
    def showtable():
        table = PrettyTable(["Номер", "Марка", "Модель", "Тип двигателя", "Цвет", "Мощность"])
        with open("cars_table.txt", "r") as t:
            S = t.read().split()
        length = (len(S) + 1) / 6
        first = 0
        last = 6
        for i in range(0, int(length)):
            table.add_row(S[first:last])
            first += 6
            last += 6
        return(print(table))
    def add_auto(count):
        '''
        Car.showtable()
        '''
        with open ("cars_table.txt", "r") as t:
            S = t.read().split()
        '''
        length = (len(S) + 1) / 6
        
        num = input("Введите регистрационный номер добавляемого автомобиля: ")
        while True:
            for i in range(0, int(length)*6, 6):
                if num == S[i]:
                    num = input("Данный регистрационный номер уже находится в базе данных.\nПожалуйста, введите другой номер: ") 
            else:
                break
        brand = input('Введите марку автомобиля: ')
        model = input('Введите модель автомобиля: ')
        engine_type = input("Введите тип двигателя автомобиля - внутреннего сгорания (1), электродвигатель (2), гибридный (3): ")
        while True:
            if engine_type != "1" and engine_type != "2" and engine_type != "3":
                print("Введено неправильное значение.\n")
                engine_type = input("Введите тип двигателя автомобиля - внутреннего сгорания (1), электродвигатель (2), гибридный (3): ")
            else:
                break
        match engine_type.split():
            case ["1"]: engine_type = "Внутреннего_сгорания"
            case ["2"]: engine_type = "Электродвигатель"
            case ["3"]: engine_type = "Гибридный"
        color = input('Введите новый цвет автомобиля: ')
        power = input('Введите новое значение мощности двигателя автомобиля: ')
        '''
        if count == 1:
            num = "43212"
            brand = "Toyota"
            model = "Harris"
            engine_type = "внутреннего_сгорания"
            color = "Серебристый"
            power = "148"
        elif count == 2:
            num = "91239"
            brand = "Honda"
            model = "CR-V"
            engine_type = "внутреннего_сгорания"
            color = "Чёрный"
            power = "125"
        S_add = [num,' ', brand,' ', model,' ', engine_type,' ', color,' ', power,' ']
        with open ("cars_table.txt", "a") as t:
            t.writelines(S_add)
        '''
        print("\nАвтомобиль успешно добавлен в базу данных.\n")
        Car.showtable()
        '''
    def change(self):
        '''
        parameter = input("Какой параметр вы хотите изменить?\n")
        '''
        parameter = "Цвет"
        match parameter.split():
            case ["Номер"]:
                inp = input("Введите новый регистрационный номер автомобиля: ")
                while True:
                    if inp in S:
                        inp = input("Данный регистрационный номер уже находится в базе данных.\nПожалуйста, введите другой номер: ") 
                    else:
                        break
                self.number = inp
                S[first] = self.number
            case ["Марка"]:
                inp = input('Введите новую марку автомобиля: ')
                self.brand = inp
                S[first+1] = self.brand
            case ["Модель"]:
                inp = input('Введите новую модель автомобиля: ')
                self.model = inp
                S[first+2] = self.model
            case ["Тип двигателя"]:
                inp = input("Введите новый тип двигателя автомобиля - внутреннего сгорания (1), электродвигатель (2), гибридный (3): ")
                while True:
                    if inp != "1" and inp != "2" and inp != "3":
                        print("Введено неправильное значение.\n")
                        inp = input("Введите новый тип двигателя автомобиля - внутреннего сгорания (1), электродвигатель (2), гибридный (3): ")
                    else:
                        break
                if inp == "1": inp = "Внутреннего сгорания"
                elif inp == "2": inp = "Электродвигатель"
                elif inp == "3": inp = "Гибридный"
                self.engine_type = inp
                S[first+3] = self.engine_type
            case ["Цвет"]:
                '''
                inp = input('Введите новый цвет автомобиля: ')
                '''
                inp = "Красный"
                self.color = inp
                S[first+4] = self.color
            case ["Мощность"]:
                inp = input('Введите новое значение мощности двигателя автомобиля: ')
                self.power = inp
                S[first+5] = self.power
                
        for i in range(0,len(S)):
            S[i]=S[i]+' '
        with open('cars_table.txt', 'w') as f:
            f.writelines(S)
    def delete(self):
        self.number = ''
        self.brand = ''
        self.model = ''
        self.engine_type = ''
        self.color = ''
        self.power = ''
        S[first] = self.number
        S[first + 1]= self.brand
        S[first + 2]=self.model
        S[first + 3]=self.engine_type
        S[first + 4]=self.color
        S[first + 5]=self.power
        for i in range(0,len(S)):
            S[i]=S[i]+' '
        with open('cars_table.txt', 'w') as f:
            f.writelines(S)


print("\nИнтеграционное тестирование - Таблица с автомобилями (консоль)")

print("\n=========================================================================================\n")


print("1 этап - вывод данных таблицы из файла")
Car.showtable()

print("\n=========================================================================================\n")



print("2 этап - занесение данных в пустой файл таблицы")
print("Заносим готовый набор данных в файл (43212, Toyota, Harris, внутреннего сгорания, Серебристый, 148).")
Car.add_auto(1)
print("Выводим таблицу - видим занесённые в файл данные.")
Car.showtable()

print("\n=========================================================================================\n")


print("3 этап - изменение данных об автомобиле в файле")
print("Изменяем цвет автомобиля, занесённого в файл (Серебристый > Красный)")
with open ("cars_table.txt", "r") as t:
                S = t.read().split()
num = "43212"
length = (len(S) + 1) / 6
first = 0
for i in range(1, int(length)):
    first += 6
car = Car(S[first], S[first + 1], S[first + 2], S[first + 3], S[first + 4], S[first + 5])
car.change()
print("Выводим таблицу - видим изменённые в файле данные.")
Car.showtable()

print("\n=========================================================================================\n")

print("4 этап - удаление данных об автомобиле из файла")
print("Добавляем в таблицу ещё один автомобиль.")
Car.add_auto(2)
Car.showtable()
print("Удаляем добавленный автомобиль.")
with open ("cars_table.txt", "r") as t:
    S = t.read().split()   
length = (len(S) + 1) / 6
first = 0
num = "91239"
for i in range(0, int(length)*6, 6):
    if num == S[i]:
        first = i
car = Car(S[first], S[first + 1], S[first + 2], S[first + 3], S[first + 4], S[first + 5])
car.delete()
Car.showtable()

print("\n=========================================================================================\n")


print("5 этап - поиск автомобиля в таблице")
print("Снова заносим автомобиль в таблицу.")
Car.add_auto(2)
Car.showtable()
print("Ищем в таблице автомобиль с номером '91239'")
with open ("cars_table.txt", "r") as t:
    S = t.read().split()
tablesearch = PrettyTable(["Номер", "Марка", "Модель", "Тип двигателя", "Цвет", "Мощность"])
searchpar = "91239"
length = (len(S) + 1) / 6
for i in range(0, int(length)*6, 6):
    if searchpar in S[i:i+6]:
        tablesearch.add_row(S[i:i+6])
        print(tablesearch)

print("\n=========================================================================================\n")

print("Очищаем файл таблицы.")

t = open ("cars_table.txt", "w")
t.truncate()