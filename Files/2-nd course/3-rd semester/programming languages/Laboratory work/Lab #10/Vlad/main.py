import tkinter as tk
from tkinter import filedialog, messagebox
from header import calculate_product_and_count_above_average

def load_from_file():
	# Открываем окно выбора файла
	filepath = filedialog.askopenfilename(filetypes=[("Text files", "*.txt")])

	if not filepath:
		return

	with open(filepath, 'r', encoding='utf-8') as f:
		content = f.read().strip()

	# Пытаемся преобразовать содержимое в список целых чисел
	try:
		numbers = list(map(int, content.split()))
		# Очищаем текстовое поле и вставляем прочитанные числа через пробел
		input_text.delete(1.0, tk.END)
		input_text.insert(tk.END, ' '.join(map(str, numbers)))
	except Exception:
		messagebox.showerror("Ошибка", "Неверный формат файла")


# Функция обработки введённых данных
def process_data():
	raw = input_text.get(1.0, tk.END).strip()

	if not raw:
		result_label.config(text="Результат: введите данные")
		return

	numbers = list(map(int, raw.split()))
	product, count = calculate_product_and_count_above_average(numbers)

	result_label.config(text=f"Результат:\nПроизведение ненулевых: {product}\nКол-во выше среднего: {count}")

# Сохранение результата вычислений в файл
def save_result():
	raw = input_text.get(1.0, tk.END).strip()

	if not raw:
		messagebox.showwarning("Предупреждение", "Нет данных для сохранения")
		return

	numbers = list(map(int, raw.split()))
	product, count = calculate_product_and_count_above_average(numbers)

	# Открываем диалог сохранения файла
	filepath = filedialog.asksaveasfilename(
		defaultextension=".txt",
		filetypes=[("Text files", "*.txt")]
	)

	if not filepath:
		return

	with open(filepath, 'w', encoding='utf-8') as f:
		f.write(f"Произведение ненулевых элементов: {product}\n")
		f.write(f"Количество элементов выше среднего: {count}\n")


# Создание главного окна приложения
root = tk.Tk()
root.title("Лабораторная работа №10 — Вариант 5")
root.geometry("600x400")
root.resizable(False, False)

# Метка над текстовым полем ввода
tk.Label(root, text="Введите числа (через пробел):").pack(pady=(10, 0))

# Текстовое поле для ввода данных
input_text = tk.Text(root, height=4, width=70)
input_text.pack(pady=5)  # Отступы сверху и снизу

# Контейнер для расположения кнопок
frame_buttons = tk.Frame(root)
frame_buttons.pack(pady=10)

# Размещение кнопок в контейнере
tk.Button(frame_buttons, text="Загрузить из файла", command=load_from_file).pack(side=tk.LEFT, padx=5)
tk.Button(frame_buttons, text="Обработать", command=process_data).pack(side=tk.LEFT, padx=5)
tk.Button(frame_buttons, text="Сохранить результат", command=save_result).pack(side=tk.LEFT, padx=5)

# Метка для вывода результата
result_label = tk.Label(
	root,
	text = "Результат:",
	justify = tk.LEFT,
	anchor = "w", # Якорь по левому краю
)
result_label.pack(pady = 10, padx = 10, anchor = "w")

# Запуск цикла обработки Tkinter
root.mainloop()