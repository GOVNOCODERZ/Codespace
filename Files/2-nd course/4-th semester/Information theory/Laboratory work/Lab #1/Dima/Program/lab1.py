import math
from collections import Counter

def get_entropy_analysis(file_name):
    try:
        with open(file_name, 'r', encoding='cp1251') as f:
            raw_text = f.read()
    except:
        with open(file_name, 'r', encoding='utf-8') as f:
            raw_text = f.read()

    text = raw_text.lower()
    
    text = text.replace('ё', 'е')
    text = text.replace('ъ', 'ь')
    
    allowed_chars = "абвгдежзийклмнопрстуфхцчшщыьэюя "
    filtered_text = "".join([char for char in text if char in allowed_chars])
    
    total_len = len(filtered_text)
    if total_len == 0:
        return "Файл пуст или не содержит русских букв."

    # 3. Подсчет частот
    counts = Counter(filtered_text)
    
    print(f"{'Символ':<10} | {'Кол-во':<8} | {'Вероятность':<12} | {'Вклад в H':<10}")
    print("-" * 50)

    total_entropy = 0
    # Сортировка по алфавиту
    for char in sorted(counts.keys()):
        count = counts[char]
        p = count / total_len
        h_i = -p * math.log2(p)
        total_entropy += h_i
        
        # Отображение символа
        char_display = repr(char) if char != " " else "'Пробел'"
        print(f"{char_display:<10} | {count:<8} | {p:.6f}     | {   h_i:.4f}")

    print("-" * 50)
    print(f"ИТОГО символов: {total_len}")
    print(f"Уникальных: {len(counts)}")
    print(f"ЭНТРОПИЯ ТЕКСТА: {total_entropy:.5f} бит/символ")

if __name__ == "__main__":
    get_entropy_analysis('input.txt')