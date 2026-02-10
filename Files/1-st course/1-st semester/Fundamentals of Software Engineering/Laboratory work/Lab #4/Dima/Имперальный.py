# Вариант с библиотеками


# from random import randint
# import os
# words = []
# with open ('slovar.txt', encoding='utf-8') as t:
#     lines = t.readlines()
#     for line in lines:
#         words.append(line.strip().split(' -'))
# #Общий цикл, в котором будут задаваться слова
# while True:
#     action = input('Введите (1) для начала игры, (0) для полного выхода из игры\nВвод: ')
#     os.system('cls')
#     match action.split():
#         case['1']:
#             worddef = words[randint(0,len(words)-1)]
#             word = worddef[0]
#             definition = worddef[1]
#             wordmass = []
#             wordgame = []

#             for letter in word:
#                 wordmass.append(letter)

#             for i in range(len(wordmass)):
#                 wordgame.append('*')
# #Цикл для слова. Содержит процесс игры
#             while True:
#                 print('-----------------Определение слова-----------------')
#                 print(definition)
#                 print('---------------------------------------------------')
#                 print(wordgame)
#                 letter = input('Введите букву: ')
#                 os.system('cls')

#                 if letter in wordmass:
#                     os.system('cls')
#                     print('Такая буква есть')
#                     for i in range(len(wordmass)):
#                         if letter == wordmass[i]:
#                             wordgame[i] = letter

#                 else:
#                     os.system('cls')
#                     print('Такой буквы нет')
#                 if wordgame == wordmass:
#                     os.system('cls')
#                     print('Слово: ', word)
#                     print('------------')
#                     print('Вы победили')
#                     print('------------')
#                     break
#         case['0']:
#             print('Вы вышли из игры')
#             break

#Вариант без библиотек

#Имперальный с системой жизни и сохранением введённых букв
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
            index = int(input('\nВведите число от 0 до 999\nВвод: '))
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
                    print('Буква', letter.upper(),'уже была введена')
                    life -= 1
                    print('Количество жизней: ',life)
                elif letter.upper() in wordmass:
                    print('Буква',letter, 'есть в списке')
                    for i in range(len(wordmass)):
                        if letter.upper() == wordmass[i]:
                            wordgame[i] = letter.upper()

                else:
                    life -= 1
                    print('Количество жизней: ',life)
                    print('Буквы', letter, 'нет')
                    
                massletters.append(letter)

                if wordgame == wordmass:
                    print('\nСлово: ', word)
                    print('------------')
                    print('Вы победили')
                    print('------------')
                    break
                if life == 0:
                    print('Вы проиграли\n')
                    break
        case['0']:
            print('Вы вышли из игры')
            break

