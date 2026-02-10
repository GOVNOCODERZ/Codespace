from tkinter import *
from tkinter.ttk import Combobox
import random
import time
import math
import os

def rand():
    abc = ab.split(' ')
    global array
    if comb2 == 'int':
        a = int(abc[0])
        b = int(abc[1])
        array = [random.randint(a, b) for i in range(int(count))]
def startfile():
    if comb1 == 'Массивом':
        os.startfile('array.txt','open')
    else:
        lbl = Label(window, text="Создайте массив")
        lbl.grid(column=0, row=4)

def write(array):
    f = open('array.txt','w+')
    f.write(str(array))
    f.truncate()
    f.close()

class Timer:
    def __init__(self):
        self._start_time = None

    def start(self):
        self._start_time = time.perf_counter()

    def stop(self):

        elapsed_time = time.perf_counter() - self._start_time
        self._start_time = None
        print(f"Вычисление заняло {elapsed_time:f} секунд")
        lbl = Label(window, text=f"Вычисление заняло {elapsed_time:f} секунд")
        lbl.grid(column=1, row=4)

class Timer2:
    def __init__(self):
        self._start_time = None
        self.sum = 0

    def start(self):
        self._start_time = time.perf_counter()

    def stop(self):

        elapsed_time = time.perf_counter() - self._start_time
        self._start_time = None
        self.sum += elapsed_time
    def print(self):
        print(self.sum)
        lbl = Label(window, text=f"Вычисление заняло {self.sum:f} секунд")
        lbl.grid(column=1, row=4)
def clicked():
    global comb1
    comb = combo.get()
    comb1 = combo1.get()
    count = txt.get()
    comb2 = combo2.get()
    ab = rand.get()
    abc = ab.split(' ')
    if comb2 == 'int':
        a = int(abc[0])
        b = int(abc[1])
        array = [random.randint(a, b) for i in range(int(count))]
    else:
        a = float(abc[0])
        b = float(abc[1])
        array = [random.uniform(a, b) for i in range(int(count))]
    timer = Timer()
    timer2 = Timer2()
    if comb == 'Сумма':

        if comb1 == 'Массивом':
            write(array)
            timer.start()
            sum = 0
            for i in range(len(array)):
                sum = sum + array[i]
            timer.stop()
            if comb2 == 'int':
                lbl = Label(window, text=f"Сумма чисел: {sum:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Сумма чисел: {sum:f}")
                lbl.grid(column=1, row=6)

        else:
            sum = 0
            if comb2 == 'int':
                timer.start()
                for i in range(int(count)):
                    sum = sum + random.randint(a,b)
                timer.stop()
                lbl = Label(window, text=f"Сумма чисел: {sum:d}")
                lbl.grid(column=1, row=6)
            else:
                timer.start()
                for i in range(int(count)):
                    sum = sum + random.uniform(a,b)
                timer.stop()
                lbl = Label(window, text=f"Сумма чисел: {sum:f}")
                lbl.grid(column=1, row=6)

    elif comb == 'Разность':

        if comb1 == 'Массивом':
            write(array)
            timer.start()
            diff = array[0]
            for i in range(1,len(array)):
                diff = diff - array[i]
            timer.stop()
            if comb2 == 'int':
                lbl = Label(window, text=f"Разность чисел: {diff:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Разность чисел: {diff:f}")
                lbl.grid(column=1, row=6)

        else:
            if comb2 == 'int':
                diff = random.randint(a, b)
                timer.start()
                for i in range(int(count)-1):
                    diff = diff - random.randint(a, b)
                timer.stop()
                lbl = Label(window, text=f"Сумма чисел: {diff:d}")
                lbl.grid(column=1, row=6)
            else:
                diff = random.uniform(a, b)
                timer.start()
                for i in range(int(count)-1):
                    diff = diff - random.uniform(a, b)
                timer.stop()
                lbl = Label(window, text=f"Сумма чисел: {diff:f}")
                lbl.grid(column=1, row=6)

    elif comb == 'Произведение':

        if comb1 == 'Массивом':
            write(array)
            timer.start()
            multi = 1
            for i in range(len(array)):
                multi = multi * array[i]
            timer.stop()
            if comb2 == 'int':
                lbl = Label(window, text=f"Произведение чисел: {multi:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Произведение чисел: {multi:f}")
                lbl.grid(column=1, row=6)

        else:
            multi = 1
            if comb2 == 'int':
                timer.start()
                for i in range(int(count)):
                    multi = multi * random.randint(a, b)
                timer.stop()
                lbl = Label(window, text=f"Произведение чисел: {multi:d}")
                lbl.grid(column=1, row=6)
            else:
                timer.start()
                for i in range(int(count)):
                   multi = multi * random.uniform(a, b)
                timer.stop()
                lbl = Label(window, text=f"Произведение чисел: {multi:f}")
                lbl.grid(column=1, row=6)

    elif comb == 'Деление':

        if comb1 == 'Массивом':
            write(array)
            timer.start()
            div = 1
            for i in range(len(array)):
                div = div / array[i]
            timer.stop()
            if comb2 == 'int':
                lbl = Label(window, text=f"Деление чисел: {div:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Деление чисел: {div:f}")
                lbl.grid(column=1, row=6)

        else:
            div = 1
            if comb2 == 'int':
                timer.start()
                for i in range(int(count)):
                    div = div / random.randint(a, b)
                timer.stop()
                lbl = Label(window, text=f"Деление чисел: {div:d}")
                lbl.grid(column=1, row=6)
            else:
                timer.start()
                for i in range(int(count)):
                    div = div / random.uniform(a, b)
                timer.stop()
                lbl = Label(window, text=f"Деление чисел: {div:f}")
                lbl.grid(column=1, row=6)

    elif comb == 'Корень квадратный':

        if comb1 == 'Массивом':
            write(array)
            sqrt_sum = 0
            for i in range(len(array)):
                timer2.start()
                sqrt = math.sqrt(array[i])
                timer2.stop()
                sqrt_sum = sqrt_sum + sqrt
            timer2.print()
            if comb2 == 'int':
                lbl = Label(window, text=f"Деление чисел: {sqrt_sum:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Деление чисел: {sqrt_sum:f}")
                lbl.grid(column=1, row=6)

        else:
            sqrt_sum = 0
            for i in range(int(count)):
                if comb2 == 'int':
                    timer2.start()
                    sqrt = math.sqrt(random.randint(a,b))
                    timer2.stop()
                    sqrt_sum = sqrt_sum + sqrt
                else:
                    timer2.start()
                    sqrt = math.sqrt(random.uniform(a, b))
                    timer2.stop()
                    sqrt_sum = sqrt_sum + sqrt
            timer2.print()
            if comb2 == 'int':
                lbl = Label(window, text=f"Деление чисел: {sqrt_sum:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Деление чисел: {sqrt_sum:f}")
                lbl.grid(column=1, row=6)

    elif comb == 'Возведение в квадрат':

        if comb1 == 'Массивом':
            write(array)
            pow_sum = 0
            for i in range(len(array)):
                timer2.start()
                pow = math.pow(array[i],2)
                timer2.stop()
                pow_sum = pow_sum + pow
            timer2.print()
            if comb2 == 'int':
                lbl = Label(window, text=f"Сумма квадратов чисел: {pow_sum:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Сумма квадратов чисел: {pow_sum:f}")
                lbl.grid(column=1, row=6)

        else:
            pow_sum = 0
            for i in range(int(count)):
                if comb2 == 'int':
                    timer2.start()
                    pow = math.pow(random.randint(a,b),2)
                    timer2.stop()
                    pow_sum =pow_sum + pow
                else:
                    timer2.start()
                    pow = math.pow(random.randint(a, b), 2)
                    timer2.stop()
                    pow_sum = pow_sum + pow
            timer2.print()
            if comb2 == 'int':
                lbl = Label(window, text=f"Сумма квадратов чисел: {pow_sum:d}")
                lbl.grid(column=1, row=6)
            else:
                lbl = Label(window, text=f"Сумма квадратов чисел: {pow_sum:f}")
                lbl.grid(column=1, row=6)


window = Tk()
window.title("Hello")

window.geometry('700x400')

combo = Combobox(window)
combo['values'] = ('Сумма', 'Разность', 'Произведение', 'Деление','Корень квадратный','Возведение в квадрат')
combo.current(0)
combo.grid(column=0, row=0)

combo1 = Combobox(window)
combo1['values'] = ('Массивом', 'Без массива')
combo1.current(0)
combo1.grid(column=0, row=1)

lbl = Label(window, text='Введите кол-во чисел')
lbl.grid(column=3, row=0)

txt = Entry(window,width=10)
txt.grid(column=3, row=1)

combo2 = Combobox(window)
combo2['values'] = ('int','float')
combo2.current(0)
combo2.grid(column=0, row=2)

lbl = Label(window, text='Введите дипозон заполнения, через пробел')
lbl.grid(column=1, row=0)

rand = Entry(window,width=10)
rand.grid(column=1, row=1)

btn = Button(window, text="Подсчёт", command=clicked)
btn.grid(column=1, row=7)

btn = Button(window, text="Вывод массива", command=startfile)
btn.grid(column=2, row=7)
window.mainloop()

