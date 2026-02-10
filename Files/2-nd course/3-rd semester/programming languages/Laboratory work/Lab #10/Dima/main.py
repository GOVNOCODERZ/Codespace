# main.py

import tkinter as tk
from tkinter import filedialog, messagebox
from logic import process_data

# --- Функции для обработки событий ---

def load_from_file():
    """Загружает данные из выбранного пользователем файла."""
    filepath = filedialog.askopenfilename(
        filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
    )
    if not filepath:
        return
    # Очищаем текстовое поле и вставляем данные из файла
    input_text.delete("1.0", tk.END)
    with open(filepath, "r") as file:
        input_text.insert("1.0", file.read())

def execute_processing():
    """Выполняет обработку данных из текстового поля."""
    try:
        # Получаем текст и преобразуем его в список чисел
        input_data = input_text.get("1.0", tk.END).strip().split()
        numbers = [int(num) for num in input_data]

        # Вызываем функцию обработки из logic.py
        min_positive, count = process_data(numbers)

        # Формируем и выводим результат
        result = (
            f"Минимальный положительный элемент: {min_positive}\n"
            f"Количество таких элементов: {count}"
        )
        output_text.delete("1.0", tk.END)
        output_text.insert("1.0", result)

    except ValueError as e:
        # Показываем ошибку, если данные некорректны
        messagebox.showerror("Ошибка ввода", str(e))
    except Exception as e:
        # Показываем другие возможные ошибки
        messagebox.showerror("Ошибка", str(e))

def save_to_file():
    """Сохраняет результат в выбранный пользователем файл."""
    filepath = filedialog.asksaveasfilename(
        defaultextension=".txt",
        filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
    )
    if not filepath:
        return
    # Записываем результат из выходного поля в файл
    with open(filepath, "w") as file:
        file.write(output_text.get("1.0", tk.END))

# --- Создание графического интерфейса ---

# Главное окно
root = tk.Tk()
root.title("Лабораторная работа №10 - Вариант 13")
root.geometry("400x450")

# Фрейм для ввода данных
input_frame = tk.Frame(root, padx=10, pady=10)
input_frame.pack(fill="x")

input_label = tk.Label(input_frame, text="Введите числа через пробел:")
input_label.pack(anchor="w")

input_text = tk.Text(input_frame, height=8, width=40)
input_text.pack(fill="x")

# Фрейм для кнопок
button_frame = tk.Frame(root, pady=5)
button_frame.pack()

load_button = tk.Button(button_frame, text="Загрузить из файла", command=load_from_file)
load_button.grid(row=0, column=0, padx=5)

process_button = tk.Button(button_frame, text="Выполнить", command=execute_processing)
process_button.grid(row=0, column=1, padx=5)

save_button = tk.Button(button_frame, text="Сохранить в файл", command=save_to_file)
save_button.grid(row=0, column=2, padx=5)

# Фрейм для вывода результата
output_frame = tk.Frame(root, padx=10, pady=10)
output_frame.pack(fill="x")

output_label = tk.Label(output_frame, text="Результат:")
output_label.pack(anchor="w")

output_text = tk.Text(output_frame, height=4, width=40)
output_text.pack(fill="x")

# Запуск главного цикла приложения
root.mainloop()