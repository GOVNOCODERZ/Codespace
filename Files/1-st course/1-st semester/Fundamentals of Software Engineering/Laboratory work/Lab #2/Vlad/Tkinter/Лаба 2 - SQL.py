import tkinter as tk
from tkinter import ttk
import sqlite3

class Main(tk.Frame):
    def __init__(self, root):
        super().__init__(root)
        self.init_main()
        self.db = db
        self.view_records()

    def init_main(self):
        toolbar = tk.Frame(bg='#d7d8e0', bd=2)
        toolbar.pack(side=tk.TOP, fill=tk.X)

        btn_open_dialog = tk.Button(toolbar, text='Добавить авто', command=self.open_dialog, bg='#d7d8e0', bd=0, compound=tk.TOP)
        btn_open_dialog.pack(side=tk.LEFT)

        btn_edit_dialog = tk.Button(toolbar, text='Редактировать', bg='#d7d8e0', bd=0, compound=tk.TOP, command=self.open_update_dialog)
        btn_edit_dialog.pack(side=tk.LEFT)

        btn_delete = tk.Button(toolbar, text='Удалить авто', bg='#d7d8e0', bd=0, compound=tk.TOP, command=self.delete_records)
        btn_delete.pack(side=tk.LEFT)

        btn_search = tk.Button(toolbar, text='Поиск', bg='#d7d8e0', bd=0, compound=tk.TOP, command=self.open_search_dialog)
        btn_search.pack(side=tk.LEFT)

        self.refresh_img = tk.PhotoImage()
        btn_refresh = tk.Button(toolbar, text='Обновить', bg='#d7d8e0', bd=0, compound=tk.TOP, command=self.view_records)
        btn_refresh.pack(side=tk.LEFT)

        self.tree = ttk.Treeview(self, columns=('Num', 'brand', 'model', 'engine_type', 'engine_volume', 'engine_power', 'colour', 'carmile', 'drive_system', 'year_of_product'), height=15, show='headings')

        self.tree.column('Num', width=50, anchor=tk.CENTER)
        self.tree.column('brand', width=130, anchor=tk.CENTER)
        self.tree.column('model', width=130, anchor=tk.CENTER)
        self.tree.column('engine_type', width=130, anchor=tk.CENTER)
        self.tree.column('engine_volume', width=130, anchor=tk.CENTER)
        self.tree.column('engine_power', width=130, anchor=tk.CENTER)        
        self.tree.column('colour', width=130, anchor=tk.CENTER)
        self.tree.column('carmile', width=130, anchor=tk.CENTER)
        self.tree.column('drive_system', width=130, anchor=tk.CENTER)
        self.tree.column('year_of_product', width=130, anchor=tk.CENTER)
        
        self.tree.heading('Num', text='Номер')
        self.tree.heading('brand', text='Марка')
        self.tree.heading('model', text='Модель авто')
        self.tree.heading('engine_type', text='Тип двигателя')
        self.tree.heading('engine_volume', text='Объём двигателя, Л')
        self.tree.heading('engine_power', text='Мощность, ЛС')
        self.tree.heading('colour', text='Цвет')
        self.tree.heading('carmile', text='Пробег, км')
        self.tree.heading('drive_system', text='Привод')
        self.tree.heading('year_of_product', text='Год выпуска')

        self.tree.pack(side=tk.LEFT)

        scroll = tk.Scrollbar(self, command=self.tree.yview)
        scroll.pack(side=tk.LEFT, fill=tk.Y)
        self.tree.configure(yscrollcommand=scroll.set)

    def records(self, brand, model, engine_type, engine_volume, engine_power, colour, carmile, drive_system, year_of_product):
        self.db.insert_data(brand, model, engine_type, engine_volume, engine_power, colour, carmile, drive_system, year_of_product)
        self.view_records()

    def update_record(self, brand, model, engine_type, engine_volume, engine_power, colour, carmile, drive_system, year_of_product):
        self.db.c.execute('''UPDATE garage SET brand=?, model=?, engine_type=?, engine_volume=?, engine_power=?, colour=?, carmile=?, drive_system=?, year_of_product=? WHERE ID=?''',
                          (brand, model, engine_type, engine_volume, engine_power, colour, carmile, drive_system, year_of_product, self.tree.set(self.tree.selection()[0], '#1')))
        self.db.conn.commit()
        self.view_records()

    def view_records(self):
        self.db.c.execute('''SELECT * FROM garage''')
        [self.tree.delete(i) for i in self.tree.get_children()]
        [self.tree.insert('', 'end', values=row) for row in self.db.c.fetchall()]

    def delete_records(self):
        for selection_item in self.tree.selection():
            self.db.c.execute('''DELETE FROM garage WHERE id=?''', (self.tree.set(selection_item, '#1'),))
        self.db.conn.commit()
        self.view_records()

    def search_records(self, brand):
        brand = ('%' + brand + '%',)
        self.db.c.execute('''SELECT * FROM garage WHERE brand LIKE ?''', brand)
        [self.tree.delete(i) for i in self.tree.get_children()]
        [self.tree.insert('', 'end', values=row) for row in self.db.c.fetchall()]

    def open_dialog(self):
        Child()

    def open_update_dialog(self):
        Update()

    def open_search_dialog(self):
        Search()


class Child(tk.Toplevel):
    def __init__(self):
        super().__init__(root)
        self.init_child()
        self.view = app

    def init_child(self):
        self.title('Добавить автомобиль')
        self.geometry('400x320+400+300')
        self.resizable(False, False)

        label_brand = tk.Label(self, text='Марка авто:')
        label_brand.place(x=50, y=20)
        label_model = tk.Label(self, text='Модель авто:')
        label_model.place(x=50, y=50)
        label_engine_type = tk.Label(self, text='Тип двигателя:')
        label_engine_type.place(x=50, y=80)
        label_engine_volume = tk.Label(self, text='Объём двигателя, Л:')
        label_engine_volume.place(x=50, y=110)
        label_engine_power = tk.Label(self, text='Мощность, ЛС:')
        label_engine_power.place(x=50, y=140)
        label_colour = tk.Label(self, text='Цвет:')
        label_colour.place(x=50, y=170)
        label_carmile = tk.Label(self, text='Пробег, КМ:')
        label_carmile.place(x=50, y=200)
        label_drive_system = tk.Label(self, text='Привод:')
        label_drive_system.place(x=50, y=230)
        label_year_of_product = tk.Label(self, text='Год выпуска:')
        label_year_of_product.place(x=50, y=260)
        
        self.entry_brand = ttk.Entry(self)
        self.entry_brand.place(x=200, y=20)
        
        self.entry_model = ttk.Entry(self)
        self.entry_model.place(x=200, y=50)
        
        self.entry_engine_type = ttk.Entry(self)
        self.entry_engine_type.place(x=200, y=80)
        
        self.entry_engine_volume = ttk.Entry(self)
        self.entry_engine_volume.place(x=200, y=110)

        self.entry_engine_power = ttk.Entry(self)
        self.entry_engine_power.place(x=200, y=140)

        self.entry_colour = ttk.Entry(self)
        self.entry_colour.place(x=200, y=170)

        self.entry_carmile = ttk.Entry(self)
        self.entry_carmile.place(x=200, y=200)

        self.entry_drive_systme = ttk.Entry(self)
        self.entry_drive_systme.place(x=200, y=230)

        self.entry_year_of_product = ttk.Entry(self)
        self.entry_year_of_product.place(x=200, y=260)

        btn_cancel = ttk.Button(self, text='Закрыть', command=self.destroy)
        btn_cancel.place(x=300, y=290)

        self.btn_ok = ttk.Button(self, text='Добавить')
        self.btn_ok.place(x=220, y=290)
        self.btn_ok.bind('<Button-1>', lambda event: self.view.records(self.entry_brand.get(),
                                                                       self.entry_model.get(),
                                                                       self.entry_engine_type.get(),
                                                                       self.entry_engine_volume.get(),
                                                                       self.entry_engine_power.get(),
                                                                       self.entry_colour.get(),
                                                                       self.entry_carmile.get(),
                                                                       self.entry_drive_systme.get(),
                                                                       self.entry_year_of_product.get()))
        self.grab_set()
        self.focus_set()


class Update(Child):
    def __init__(self):
        super().__init__()
        self.init_edit()
        self.view = app
        self.db = db
        self.default_data()

    def init_edit(self):
        self.title('Редактировать авто')
        btn_edit = ttk.Button(self, text='Редактировать')
        btn_edit.place(x=205, y=290)
        btn_edit.bind('<Button-1>', lambda event: self.view.update_record(self.entry_brand.get(),
                                                                       self.entry_model.get(),
                                                                       self.entry_engine_type.get(),
                                                                       self.entry_engine_volume.get(),
                                                                       self.entry_engine_power.get(),
                                                                       self.entry_colour.get(),
                                                                       self.entry_carmile.get(),
                                                                       self.entry_drive_systme.get(),
                                                                       self.entry_year_of_product.get()))
        self.btn_ok.destroy()

    def default_data(self):
        self.db.c.execute('''SELECT * FROM garage WHERE id=?''',
                          (self.view.tree.set(self.view.tree.selection()[0], '#1'),))



class Search(tk.Toplevel):
    def __init__(self):
        super().__init__()
        self.init_search()
        self.view = app

    def init_search(self):
        self.title('Поиск')
        self.geometry('300x130+400+300')
        self.resizable(False, False)

        label_search = tk.Label(self, text='Поиск')
        label_search.place(x=50, y=20)

        self.entry_search = ttk.Entry(self)
        self.entry_search.place(x=105, y=20, width=150)

        btn_cancel = ttk.Button(self, text='Закрыть', command=self.destroy)
        btn_cancel.place(x=185, y=50)

        btn_search = ttk.Button(self, text='Поиск')
        btn_search.place(x=105, y=50)
        btn_search.bind('<Button-1>', lambda event: self.view.search_records(self.entry_search.get()))
        btn_search.bind('<Button-1>', lambda event: self.destroy(), add='+')


class DB:
    def __init__(self):
        self.conn = sqlite3.connect('laba 2 database.db')
        self.c = self.conn.cursor()
        self.c.execute(
            '''CREATE TABLE IF NOT EXISTS garage (id integer primary key, brand text, model text, engine_type text, engine_volume text, engine_power text, colour text, carmile text, drive_system text, year_of_product text)''')
        self.conn.commit()

    def insert_data(self, brand, model, engine_type, engine_volume, engine_power, colour, carmile, drive_system, year_of_product):
        self.c.execute('''INSERT INTO garage(brand, model, engine_type, engine_volume, engine_power, colour, carmile, drive_system, year_of_product) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)''',
                       (brand, model, engine_type, engine_volume, engine_power, colour, carmile, drive_system, year_of_product))
        self.conn.commit()


if __name__ == "__main__":
    root = tk.Tk()
    db = DB()
    app = Main(root)
    app.pack()
    root.title("garage")
    root.geometry("1250x450+300+200")
    root.resizable(False, False)
    root.mainloop()