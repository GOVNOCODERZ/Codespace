def swap_adjacent_words_in_line(line):
    words = line.split() # Разбиваем строку на слова по пробелам и запятым

    for i in range(0, len(words) - 1, 2): # Проходим по парам слов
        # Меняем местами текущее слово и следующее (чётное с нечётным)
        words[i], words[i + 1] = words[i + 1], words[i]

    return " ".join(words) # Собираем слова в строку и возвращаем


def process_text(text):
    lines = text.splitlines() # Разбиваем текст на строки

    # Применяем перестановку к каждой строке
    processed_lines = [swap_adjacent_words_in_line(line) for line in lines]

    # Объединяем строки и возвращаем
    return "\n".join(processed_lines)