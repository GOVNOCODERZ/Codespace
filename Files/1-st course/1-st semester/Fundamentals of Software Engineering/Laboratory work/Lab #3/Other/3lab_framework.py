import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import tabulate

columns = ['№', 'Производитель', 'Марка', 'Цвет', 'Коробка передач', 'Привод']

def showdata():
    with open('data_source.txt', 'r+', encoding='utf-16') as file:
        data = file.readlines()
        r = []
        for line in data:
            s = line.strip().split('/')
            r.append(s)
        return r

def find_num():
    data = showdata()
    number = 0
    for line in data:
        num1 = int(line[0])
        if num1 > number:
            number = num1
    return number

def delete_car(number_for_delete):
    data = showdata()
    with open('data_source.txt', 'w+', encoding='utf-16') as file:
        rdata = []
        for i, line in enumerate(data):
            if int(line[0]) != number_for_delete:
                rdata.append(line)
        for i in range(len(rdata)):
            rdata[i][0] = str(i + 1)
        s = ["/".join(x) for x in rdata]
        file.write("\n".join(s))
        update_data()

def change_car(number_for_change):
    data = showdata()
    for i, line in enumerate(data):
        if int(line[0]) == number_for_change:
            r = [line]
            break
    else:
        messagebox.showinfo("Ошибка", "Машины с таким номером нет")
        return

    def change_value(column_index, new_value):
        data[number_for_change - 1][column_index] = new_value
        with open('data_source.txt', 'w+', encoding='utf-16') as file:
            s = ["/".join(x) for x in data]
            file.write("\n".join(s))
        update_data()

    change_window = tk.Toplevel(root)
    change_window.title(f"Редактирование машины №{number_for_change}")

    print(tabulate.tabulate(r, headers=columns, tablefmt='pipe'))
    for i, column in enumerate(columns):
        label = tk.Label(change_window, text=f"{column}:")
        label.grid(row=i, column=0, padx=5, pady=5)
        entry = tk.Entry(change_window)
        entry.insert(0, r[0][i])
        entry.grid(row=i, column=1, padx=5, pady=5)
        entry.bind("<Return>", lambda event, index=i: change_value(index, entry.get()))

    update_data()

def add_car():
    def add_car_to_file():
        fabric = fabric_entry.get()
        model = model_entry.get()
        color = color_entry.get()
        transmission = transmission_entry.get()
        engine = engine_entry.get()
        if any([x.count('/') > 0 for x in [fabric, model, color, transmission, engine]]):
            messagebox.showinfo("Ошибка", "Введен запрещенный символ: /")
            return
        num = find_num() + 1
        car_string = f'{num}/{fabric}/{model}/{color}/{transmission}/{engine}'
        with open('data_source.txt', 'a+', encoding='utf-16') as file:
            file.write(f"{car_string}\n")
        add_car_window.destroy()
        update_data()

    add_car_window = tk.Toplevel(root)
    add_car_window.title("Добавить машину")

    fabric_label = tk.Label(add_car_window, text="Производитель:")
    fabric_label.grid(row=0, column=0, padx=5, pady=5)
    fabric_entry = tk.Entry(add_car_window)
    fabric_entry.grid(row=0, column=1, padx=5, pady=5)

    model_label = tk.Label(add_car_window, text="Марка:")
    model_label.grid(row=1, column=0, padx=5, pady=5)
    model_entry = tk.Entry(add_car_window)
    model_entry.grid(row=1, column=1, padx=5, pady=5)

    color_label = tk.Label(add_car_window, text="Цвет:")
    color_label.grid(row=2, column=0, padx=5, pady=5)
    color_entry = tk.Entry(add_car_window)
    color_entry.grid(row=2, column=1, padx=5, pady=5)

    transmission_label = tk.Label(add_car_window, text="Коробка передач:")
    transmission_label.grid(row=3, column=0, padx=5, pady=5)
    transmission_entry = tk.Entry(add_car_window)
    transmission_entry.grid(row=3, column=1, padx=5, pady=5)

    engine_label = tk.Label(add_car_window, text="Привод:")
    engine_label.grid(row=4, column=0, padx=5, pady=5)
    engine_entry = tk.Entry(add_car_window)
    engine_entry.grid(row=4, column=1, padx=5, pady=5)

    add_button = tk.Button(add_car_window, text="Добавить", command=add_car_to_file)
    add_button.grid(row=5, column=0, columnspan=2, padx=5, pady=5)

def find_car():
    def find_car_by_number():
        try:
            number_for_find = int(find_entry.get())
        except ValueError:
            messagebox.showinfo("Ошибка", "Введите корректный номер машины")
            return
        data = showdata()
        for i, line in enumerate(data):
            if int(line[0]) == number_for_find:
                r = [line]
                break
        else:
            messagebox.showinfo("Ошибка", "Машины с таким номером нет")
            return

        find_window = tk.Toplevel(root)
        find_window.title(f"Информация о машине №{number_for_find}")

        print(tabulate.tabulate(r, headers=columns, tablefmt='pipe'))
        for i, column in enumerate(columns):
            label = tk.Label(find_window, text=f"{column}: {r[0][i]}")
            label.pack()

    find_window = tk.Toplevel(root)
    find_window.title("Поиск машины")

    find_label = tk.Label(find_window, text="Введите номер машины:")
    find_label.pack()
    find_entry = tk.Entry(find_window)
    find_entry.pack()

    find_button = tk.Button(find_window, text="Найти", command=find_car_by_number)
    find_button.pack()

def update_data():
    data = showdata()
    tree.delete(*tree.get_children())
    for i, line in enumerate(data):
        tree.insert("", "end", values=line)

root = tk.Tk()
root.title("База данных машин")

tree = ttk.Treeview(root, columns=columns, show='headings')
for column in columns:
    tree.column(column, width=100, anchor='center')
    tree.heading(column, text=column)

tree.pack()

menubar = tk.Menu(root)

filemenu = tk.Menu(menubar, tearoff=0)
filemenu.add_command(label="Добавить машину", command=add_car)
filemenu.add_separator()
filemenu.add_command(label="Выход", command=root.quit)
menubar.add_cascade(label="Файл", menu=filemenu)

editmenu = tk.Menu(menubar, tearoff=0)
editmenu.add_command(label="Удалить машину", command=lambda: delete_car(int(tree.item(tree.selection()[0])['values'][0])))
editmenu.add_command(label="Редактировать машину", command=lambda: change_car(int(tree.item(tree.selection()[0])['values'][0])))
menubar.add_cascade(label="Редактировать", menu=editmenu)

searchmenu = tk.Menu(menubar, tearoff=0)
searchmenu.add_command(label="Найти машину", command=find_car)
menubar.add_cascade(label="Поиск", menu=searchmenu)

root.config(menu=menubar)

update_data()

root.mainloop()