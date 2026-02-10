def check():
    # Функция проверки вводимых данных
    while True:
        try:
            index = int(input('Введите число от 0 до 999\nВвод:  '))
            testmass = ['*']*999
            testword = testmass[index]
            break
        except TypeError:
            print('Вы ввели символ неверного типа')
        except ValueError:
            print('Число введено неверно')
        except NameError:
            print('Тип введённых символов не подходит')
        except IndexError:
            print('Число не входит в промежуток [0;999]')
    return index

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
            index = check()
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

