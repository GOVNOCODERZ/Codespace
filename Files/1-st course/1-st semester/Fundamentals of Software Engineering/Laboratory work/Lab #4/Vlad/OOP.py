'''
Импортируем словарь в переменную
Выбираем слово из словаря
Раскидываем слово по буквам в массив, описание отдельно
Юзер вводит букву или целое слово
ПРОВЕРКИ НА ДУРАКА
Угадал - плюс очко
Играть дальше/выйти
При выходе показываем набранный счёт
'''
from random import *

class Game():
    def word_pick():
        global dictionary
        number = randint(0, 999)
        dict_word = dictionary[number] #Строка из слова + описания
        word, definition = dict_word[0], dict_word[1] #Слово и описание отдельно
        word = word.upper()
        definition = definition[1:len(definition)].capitalize()
        return word, definition

count = 0
dictionary = []
with open("dictionary.txt", encoding="utf-8") as d:
    lines = d.readlines()
    for line in lines:
        dictionary.append(line.strip().split(' -'))

print("==================ПОЛЕ ЧУДЕС==================")
while True:
    print(f"Слов отгадано: {count}\n")
    menu = input("Выберите опцию:\n1 - Играть\n2 - Выйти из игры\nВвод: ")
    match menu.split():
        case ["1"]:
            word, definition = Game.word_pick()
            word_letters = list(word)
            guess_word = ["*"] * len(word)
            
            while True:
                print("\n==========Угадайте слово==========")
                print(guess_word)
                print("\n===========Определение===========")
                print(definition)
                print("\n1 - показать загаданное слово")
                user_input = input("\nВведите букву или слово целиком: ")
                
                if user_input.upper() == word:
                    print(f"\nПравильный ответ!\nЗагаданное слово - '{word}'")
                    count += 1
                    break
                elif user_input == "1":
                    print(f"\nВы не угадали слово\nЗагаданное слово - '{word}'")
                    break
                elif user_input.upper() in word_letters:
                    print(f"\nВы угадали букву '{user_input.upper()}'")
                    for i in range(len(word_letters)):
                        if word_letters[i] == user_input.upper():
                            guess_word[i] = user_input.upper()
                else:
                    print("\nВы ничего не угадали")
                
                if word_letters == guess_word:
                    print(f"\nСлово отгадано!\nЗагаданное слово - '{word}'")
                    count += 1
                    break
        case ["2"]:
            print("\nВы вышли из игры")
            break
        case _:
            print("\nКоманда не распознана\n")