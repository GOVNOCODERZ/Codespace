from prettytable import PrettyTable
class Car():
    def __init__(self, number, mark, model, yearofmodel, color, hp, carmile,owners,crushes):
        self.number = number
        self.mark = mark
        self.model = model
        self.yearofmodel = yearofmodel
        self.color = color
        self.hp = hp
        self.carmile = carmile
        self.owners = owners
        self.crushes = crushes
    def inittable():
        table = PrettyTable(["Номер", "Марка", "Модель", "Год выпуска", "Цвет", "Лошадиные силы", "Пробег, км", "Количество владельцев", "Количество аварий"])
        with open("testgarage.txt", "r") as t:
            S = t.read().split()
        length = (len(S) + 1) / 9
        first = 0
        last = 9
        for i in range(0, int(length)):
            table.add_row(S[first:last])
            first += 9
            last += 9
        return(print(table))
    def change(self):
        self.mark = 'BMW'
        S[first+1] = self.mark
        self.model = 'M3'
        S[first+2] = self.model
        self.yearofmodel = '1995'
        S[first+3] = self.yearofmodel 
        self.colour = 'black'
        S[first+4] = self.colour
        self.hp = '200'
        S[first+5] = self.hp
        self.carmile = '151234'
        S[first+6] = self.carmile 
        self.owners = '2'
        S[first+7] = self.owners 
        self.crushes = '1'
        S[first+8] = self.crushes 
        for i in range(0,len(S)):
            S[i]=S[i]+' '
        with open('testgarage.txt', 'w') as f:
            f.writelines(S)
    def delete(self):
        self.number = ''
        self.mark = ''
        self.model = ''
        self.yearofmodel = ''
        self.color = ''
        self.hp  = ''
        self.carmile = ''
        self.owners = ''
        self.crushes = ''
        S[first] = self.number
        S[first + 1]= self.mark
        S[first + 2]=self.model
        S[first + 3]=self.yearofmodel
        S[first + 4]=self.color
        S[first + 5]=self.hp
        S[first + 6]=self.carmile
        S[first + 7]=self.owners
        S[first + 8]=self.crushes
        for i in range(0,len(S)):
            S[i]=S[i]+' '
        with open('testgarage.txt', 'w') as f:
            f.writelines(S)

        


print('----------------Тест таблицы----------------')
Car.inittable()
print('--------------------------------------------\n')
print('----------------Тест добавления машины----------------')
S_add = ['1',' ', 'Toyota',' ', 'MarkII',' ', '2000',' ', 'white',' ', '125',' ', '10000',' ', '3',' ', '0',' ']
with open ("testgarage.txt", "a") as t:
    t.writelines(S_add)
Car.inittable()
print('------------------------------------------------------\n')
print('----------------Тест редактирования----------------')
with open ("testgarage.txt", "r") as t:
    S = t.read().split()
num = '1'
length = (len(S) + 1) / 9
first = 0
while True:
    for i in range(0, int(length)*9, 9):
        if num == S[i]:
            first = i
            break
    if num == S[i]:
        break

car = Car(S[first], S[first + 1], S[first + 2], S[first + 3], S[first + 4], S[first + 5], S[first + 6], S[first + 7], S[first + 8])
car.change()
with open('testgarage.txt', 'w') as f:
    f.writelines(S)
Car.inittable()
print('---------------------------------------------------\n')
print('----------------Тест удаления----------------')
with open ("testgarage.txt", "r") as t:
    S = t.read().split()
length = (len(S) + 1) / 9
first = 0
num = '1'
while True:
    for i in range(0, int(length)*9, 9):
        if num == S[i]:
            first = i
            break
    if num == S[i]:
        break
car = Car(S[first], S[first + 1], S[first + 2], S[first + 3], S[first + 4], S[first + 5], S[first + 6], S[first + 7], S[first + 8])
car.delete()
Car.inittable()
print('---------------------------------------------\n')
