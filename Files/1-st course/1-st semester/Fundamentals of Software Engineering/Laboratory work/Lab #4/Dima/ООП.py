from random import randint
import os
class Game():
    def chancedef():
        chance = randint(0,10)
        if chance == 5:
            print("⣿⣿⣿⣿⣿⣿⣿⠿⠿⢛⣋⣙⣋⣩⣭⣭⣭⣭⣍⣉⡛⠻⢿⣿⣿⣿⣿\n\
⣿⣿⣿⠟⣋⣥⣴⣾⣿⣿⣿⡆⣿⣿⣿⣿⣿⣿⡿⠟⠛⠗⢦⡙⢿⣿⣿\n\
⣿⡟⡡⠾⠛⠻⢿⣿⣿⣿⡿⠃⣿⡿⣿⠿⠛⠉⠠⠴⢶⡜⣦⡀⡈⢿⣿\n\
⡿⢀⣰⡏⣼⠋⠁⢲⡌⢤⣠⣾⣷⡄⢄⠠⡶⣾⡀⠀⣸⡷⢸⡷⢹⠈⣿\n\
⡇⢘⢿⣇⢻⣤⣠⡼⢃⣤⣾⣿⣿⣿⢌⣷⣅⡘⠻⠿⢛⣡⣿⠀⣾⢠⣿\n\
⣷⠸⣮⣿⣷⣨⣥⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⢁⡼⠃⣼⣿\n\
⡟⠛⠛⠛⣿⠛⠛⢻⡟⠛⠛⢿⡟⠛⠛⡿⢻⡿⠛⡛⢻⣿⠛⡟⠛⠛⢿\n\
⡇⢸⣿⠀⣿⠀⠛⢻⡇⠸⠃⢸⡇⠛⢛⡇⠘⠃⢼⣷⡀⠃⣰⡇⠸⠇⢸\n\
⡇⢸⣿⠀⣿⠀⠛⢻⡇⢰⣿⣿⡇⠛⠛⣇⢸⣧⠈⣟⠃⣠⣿⡇⢰⣾⣿\n\
⣿⣿⣿⠘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢋⣿⠙⣷⢸⣷⠀⣿⣿⣿\n\
⣿⣿⣿⡇⢻⣿⣿⣿⡿⠿⢿⣿⣿⣿⠟⠋⣡⡈⠻⣇⢹⣿⣿⢠⣿⣿⣿\n\
⣿⣿⣿⣿⠘⣿⣿⣿⣿⣯⣽⣉⣿⣟⣛⠷⠙⢿⣷⣌⠀⢿⡇⣼⣿⣿⣿\n\
⣿⣿⣿⡿⢀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣤⡙⢿⢗⣀⣁⠈⢻⣿⣿\n\
⣿⡿⢋⣴⣿⣎⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡉⣯⣿⣷⠆⠙⢿\n\
⣏⠀⠈⠧⠡⠉⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃⠉⢉⣁⣀⣀⣾\n")
        
    words = []
    with open ('slovar.txt', encoding='utf-8') as t:
        lines = t.readlines()
        for line in lines:
            words.append(line.strip().split(' -'))
    #Общий цикл, в котором будут задаваться слова
    while True:
        action = input('Введите (1) для начала игры, (0) для полного выхода из игры\nВвод: ')
        match action.split():
            case['1']:
                index = randint(0,len(words)-1)
                worddef = words[index]
                word = worddef[0]
                word = word.upper()
                definition = worddef[1]
                wordmass = []
                wordgame = []

                life = 10
                massletters = []


                for letter in word:
                    wordmass.append(letter)

                for i in range(len(wordmass)):
                    wordgame.append('*')
    #Цикл для слова. Содержит процесс игры
                while True:
                    print('\n-----------------Определение слова-----------------')
                    print(definition)
                    print('---------------------------------------------------\n')
                    print(wordgame)
                    print('Количество жизней: ', life)
                    letter = input('\nВведите букву или слово целиком: ')
                    if letter.upper() == word:
                        print('\nСлово: ', word)
                        print('------------')
                        print('Вы победили')
                        print('------------')
                        break
                    elif letter.upper() in massletters:
                        os.system('cls')
                        print('Буква', letter.upper(),'уже была введена')
                        life -= 1
                        print('Количество жизней: ',life)
                    elif letter.upper() in wordmass:
                        print('Буква',letter, 'есть в списке')
                        for i in range(len(wordmass)):
                            if letter.upper() == wordmass[i]:
                                wordgame[i] = letter.upper()

                    else:
                        os.system('cls')
                        life -= 1
                        print('Буквы', letter, 'нет')
                        
                    massletters.append(letter)

                    if wordgame == wordmass:
                        os.system('cls')
                        print('\nСлово: ', word)
                        print('------------')
                        print('Вы победили')
                        print('------------')
                        break
                    if life == 0:
                        chancedef()
                        print('Вы проиграли\n')
                        print(f'Слово: {word}\n')
                        break
            case['0']:
                print('Вы вышли из игры')
                break

