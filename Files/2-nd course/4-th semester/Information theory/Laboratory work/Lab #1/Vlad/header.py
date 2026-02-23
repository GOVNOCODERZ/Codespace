import math
from collections import Counter

class TextEntropyAnalyzer:
    """Анализатор энтропии текста.
    """
    def __init__(self):
        self.__filepath = "" # Файл с текстом
        self.__text = "" # Текст для обработки
        
        self.__char_counts = Counter() # Количество каждого символа
        self.__letters_probabilities = {} # Вероятность каждого символа
        
        self.__total_chars = 0 # Общее количество символов
        self.__entropy = 0.0 # Энтропия системы
        
        self.__is_analyzed = False # Флаг состояния
        
    def load_file(self, filepath):
        """Считывает текст из файла.

        Args:
            filepath (string): Название файла

        Returns:
            boolean: Результат загрузки
        """
        try:
            with open(filepath, 'r', encoding='utf-8') as file:
                self.__text = file.read()
                
            self.__filepath = filepath
            self.__is_analyzed = False
            print(f"Файл {filepath} успешно загружен.")
            return True
        except FileNotFoundError:
            print(f"Файл {filepath} не найден.")
            return False
        except Exception as e:
            print(f"Ошибка при чтении файла: {e}")
            return False
    
    def analyze(self):
        """
        Анализирует загруженный текст, высчитывая энтропию текста,
        вероятность каждого символа и их общее количество.

        Returns:
            boolean: Результат обработки
        """
        if not self.__text:
            print("ОШИБКА: Входной текст пуст или файл не загружен.")
            return False
        
        # Обработка текста
        clear_text = self.__text.lower()
        self.__total_chars = len(clear_text)
        self.__char_counts = Counter(clear_text)
        self.__letters_probabilities = {}

        # Рассчёт энтропии
        entropy_sum = 0.0 # Суммарная энтропия
        for char, count in self.__char_counts.items():
            p = count / self.__total_chars # Вероятность символа
            self.__letters_probabilities[char] = p
            
            entropy_sum += -(p * math.log2(p)) # += Энтропия символа
        
        self.__entropy = entropy_sum
        self.__is_analyzed = True
        print("Анализ текста завершён.")

    def print_probability_table(self, limit=None):
        """Форматированный вывод таблицы вероятностей в консоль.

        Args:
            limit (int, опционально): Количество строк для вывода. None — все строки.
        """
        
        if not self.__is_analyzed:
            print("ОШИБКА: Текст ещё не проанализирован!")
            return

        # Сортировка: по убыванию вероятности (-item[1]), затем по алфавиту (item[0])
        sorted_items = sorted(self.__letters_probabilities.items(), key=lambda item: (-item[1], item[0]))

        # Если задан лимит, обрезаем список
        if limit is not None:
            data_to_print = sorted_items[:limit]
            title = f"ТОП-{limit} СИМВОЛОВ"
        else:
            data_to_print = sorted_items
            title = "ПОЛНАЯ ТАБЛИЦА ВЕРОЯТНОСТЕЙ"

        print("\n" + "=" * 50)
        print(f"{title:^50}")
        print("=" * 50)
        print(f"{'Символ':<10} | {'Кол-во':<10} | {'Вероятность':<15}")
        print("-" * 50)

        for char, prob in data_to_print:
            count = self.__char_counts[char]
            print(f"{repr(char):<10} | {count:<10} | {prob:.6f}") # repr() нужен для отображения спецсимволов (\n, \t)
        
        print("-" * 50)
        
    def print_report(self):
        """Выводит краткий отчёт по результатам анализа.
        """
        
        if not self.__is_analyzed:
            print("ОШИБКА: Текст ещё не проанализирован!")
            return

        print("\n" + "="*50)
        print(f"ОТЧЕТ ПО ФАЙЛУ: {self.__filepath}")
        print("="*50)
        print(f"Всего символов:      {self.__total_chars}")
        print(f"Уникальных символов: {len(self.__char_counts)}")
        print(f"ЭНТРОПИЯ ТЕКСТА:     {self.__entropy:.4f} бит/символ")
        # Выводим 10 самых частых символов для наглядности.
        self.print_probability_table(limit=10) 
