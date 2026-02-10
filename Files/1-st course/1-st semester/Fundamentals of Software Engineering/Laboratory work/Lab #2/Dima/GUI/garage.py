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

        self.tree = ttk.Treeview(self, columns=('Num', 'mark', 'model', 'yearofmodel','colour','hp','probeg','owners','crushes'), height=15, show='headings')

        self.tree.column('Num', width=60, anchor=tk.CENTER)
        self.tree.column('mark', width=100, anchor=tk.CENTER)
        self.tree.column('model', width=100, anchor=tk.CENTER)
        self.tree.column('yearofmodel', width=100, anchor=tk.CENTER)
        self.tree.column('colour', width=100, anchor=tk.CENTER)
        self.tree.column('hp', width=100, anchor=tk.CENTER)
        self.tree.column('probeg', width=100, anchor=tk.CENTER)
        self.tree.column('owners', width=100, anchor=tk.CENTER)
        self.tree.column('crushes', width=100, anchor=tk.CENTER)
        
        self.tree.heading('Num', text='Номер')
        self.tree.heading('mark', text='Марка')
        self.tree.heading('model', text='Модель авто')
        self.tree.heading('yearofmodel', text='Год выпуска')
        self.tree.heading('colour', text='Цвет')
        self.tree.heading('hp', text='ЛС')
        self.tree.heading('probeg', text='Пробег')
        self.tree.heading('owners', text='Кол-во владельцев')
        self.tree.heading('crushes', text='Кол-во аварий')

        self.tree.pack(side=tk.LEFT)

        scroll = tk.Scrollbar(self, command=self.tree.yview)
        scroll.pack(side=tk.LEFT, fill=tk.Y)
        self.tree.configure(yscrollcommand=scroll.set)

    def records(self, mark, model, yearofmodel, colour, hp, probeg, owners, crushes):
        self.db.insert_data(mark, model, yearofmodel, colour, hp, probeg, owners, crushes)
        self.view_records()

    def update_record(self, mark, model, yearofmodel, colour, hp, probeg, owners, crushes):
        self.db.c.execute('''UPDATE garage SET mark=?, model=?, yearofmodel=?, colour=?, hp=?, probeg=?, owners=?, crushes=? WHERE ID=?''',
                          (mark, model, yearofmodel, colour, hp, probeg, owners, crushes, self.tree.set(self.tree.selection()[0], '#1')))
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

    def search_records(self, mark):
        mark = ('%' + mark + '%',)
        self.db.c.execute('''SELECT * FROM garage WHERE mark LIKE ?''', mark)
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

        label_mark = tk.Label(self, text='Марка авто:')
        label_mark.place(x=50, y=20)
        label_model = tk.Label(self, text='Модель авто:')
        label_model.place(x=50, y=50)
        label_yearofmodel = tk.Label(self, text='Год выпуска:')
        label_yearofmodel.place(x=50, y=80)
        label_colour = tk.Label(self, text='Цвет:')
        label_colour.place(x=50, y=110)
        label_hp = tk.Label(self, text='Лошадиные силы:')
        label_hp.place(x=50, y=140)
        label_probeg = tk.Label(self, text='Пробег, км:')
        label_probeg.place(x=50, y=170)
        label_owners = tk.Label(self, text='Кол-во владельцев:')
        label_owners.place(x=50, y=200)
        label_crushes = tk.Label(self, text='Кол-во аварий:')
        label_crushes.place(x=50, y=230)
        
        self.entry_mark = ttk.Entry(self)
        self.entry_mark.place(x=200, y=20)
        
        self.entry_model = ttk.Entry(self)
        self.entry_model.place(x=200, y=50)
        
        self.entry_yearofmodel = ttk.Entry(self)
        self.entry_yearofmodel.place(x=200, y=80)
        
        self.entry_colour = ttk.Entry(self)
        self.entry_colour.place(x=200, y=110)

        self.entry_hp = ttk.Entry(self)
        self.entry_hp.place(x=200, y=140)

        self.entry_probeg = ttk.Entry(self)
        self.entry_probeg.place(x=200, y=170)
        
        self.entry_owners = ttk.Entry(self)
        self.entry_owners.place(x=200, y=200)
        
        self.entry_crushes = ttk.Entry(self)
        self.entry_crushes.place(x=200, y=230)

        btn_cancel = ttk.Button(self, text='Закрыть', command=self.destroy)
        btn_cancel.place(x=300, y=260)

        self.btn_ok = ttk.Button(self, text='Добавить')
        self.btn_ok.place(x=220, y=260)
        self.btn_ok.bind('<Button-1>', lambda event: self.view.records(self.entry_mark.get(),
                                                                       self.entry_model.get(),
                                                                       self.entry_yearofmodel.get(),
                                                                       self.entry_colour.get(),
                                                                       self.entry_hp.get(),
                                                                       self.entry_probeg.get(),
                                                                       self.entry_owners.get(),
                                                                       self.entry_crushes.get()))

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
        btn_edit.place(x=205, y=260)
        btn_edit.bind('<Button-1>', lambda event: self.view.update_record(self.entry_mark.get(),
                                                                       self.entry_model.get(),
                                                                       self.entry_yearofmodel.get(),
                                                                       self.entry_colour.get(),
                                                                       self.entry_hp.get(),
                                                                       self.entry_probeg.get(),
                                                                       self.entry_owners.get(),
                                                                       self.entry_crushes.get()))

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
        self.geometry('300x100+400+300')
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
        self.conn = sqlite3.connect('garage.db')
        self.c = self.conn.cursor()
        self.c.execute(
            '''CREATE TABLE IF NOT EXISTS garage (id integer primary key, mark text, model text, Yearofmodel text, colour text, hp text, probeg text, owners text, crushes text)''')
        self.conn.commit()

    def insert_data(self, mark, model, yearofmodel, colour, hp, probeg, owners, crushes):
        self.c.execute('''INSERT INTO garage(mark, model, Yearofmodel, colour, hp, probeg, owners, crushes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)''',
                       (mark, model, yearofmodel, colour, hp, probeg, owners, crushes))
        self.conn.commit()


if __name__ == "__main__":
    root = tk.Tk()
    db = DB()
    app = Main(root)
    app.pack()
    root.title("garage")
    root.geometry("900x360")
    root.resizable(False, False)
    root.mainloop()