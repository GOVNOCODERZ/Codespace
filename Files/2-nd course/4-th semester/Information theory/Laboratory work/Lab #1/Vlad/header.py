import math
from collections import Counter

class TextEntropyAnalyzer:
    """Анализатор энтропии текста.
    """
    def __init__(self):
        self._filepath = "" # Файл с текстом
        self._text = "" # Текст для обработки
        
        self._char_counts = Counter() # Количество каждого символа
        self._letters_probabilities = {} # Вероятность каждого символа
        
        self._total_chars = 0 # Общее количество символов
        self._entropy = 0.0 # Энтропия системы
        
        self._is_analyzed = False # Флаг состояния
        
    def load_file(self, filepath):
        """Считывает текст из файла.

        Args:
            filepath (string): Название файла

        Returns:
            bool: Результат загрузки
        """
        try:
            with open(filepath, 'r', encoding='utf-8') as file:
                self._text = file.read()
                
            self._filepath = filepath
            self._is_analyzed = False
            return True
        except FileNotFoundError:
            raise FileNotFoundError(f"Файл '{filepath}' не найден.")
        except Exception as e:
            raise IOError(f"Ошибка при чтении файла: {e}") from e
    
    def analyze(self):
        """
        Анализирует загруженный текст, высчитывая энтропию текста,
        вероятность каждого символа и их общее количество.

        Returns:
            bool: Результат анализа
        """
        if not self._text:
            raise ValueError("Входной текст пуст или файл не загружен.")
        
        # Обработка текста
        clear_text = self._text.lower()
        self._total_chars = len(clear_text)
        self._char_counts = Counter(clear_text)
        self._letters_probabilities = {}

        # Рассчёт энтропии
        entropy_sum = 0.0 # Суммарная энтропия
        for char, count in self._char_counts.items():
            p = count / self._total_chars # Вероятность символа
            self._letters_probabilities[char] = p
            
            entropy_sum += -(p * math.log2(p)) # += Энтропия символа
        
        self._entropy = entropy_sum
        self._is_analyzed = True
        return True

    def print_probability_table(self, limit=None):
        """Форматированный вывод таблицы вероятностей в консоль.

        Args:
            limit (int, опционально): Количество строк для вывода. None — все строки.
        """
        
        if not self._is_analyzed:
            raise RuntimeError("Текст ещё не проанализирован!")

        # Сортировка: по убыванию вероятности (-item[1]), затем по алфавиту (item[0])
        sorted_items = sorted(self._letters_probabilities.items(), key=lambda item: (-item[1], item[0]))

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
            count = self._char_counts[char]
            print(f"{repr(char):<10} | {count:<10} | {prob:.6f}") # repr() нужен для отображения спецсимволов (\n, \t)
        
        print("-" * 50)
        
    def print_report(self):
        """Выводит краткий отчёт по результатам анализа.
        """
        
        if not self._is_analyzed:
            print("ОШИБКА: Текст ещё не проанализирован!")
            return

        print("\n" + "="*50)
        print(f"ОТЧЕТ ПО ФАЙЛУ: {self._filepath}")
        print("="*50)
        print(f"Всего символов:      {self._total_chars}")
        print(f"Уникальных символов: {len(self._char_counts)}")
        print(f"ЭНТРОПИЯ ТЕКСТА:     {self._entropy:.4f} бит/символ")
        # Выводим 10 самых частых символов для наглядности.
        self.print_probability_table(limit=10) 
