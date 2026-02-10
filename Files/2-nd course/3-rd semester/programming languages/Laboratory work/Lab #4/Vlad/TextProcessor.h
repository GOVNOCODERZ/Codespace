#ifndef TEXTPROCESSOR_H
#define TEXTPROCESSOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <set>
#include <iomanip>

using namespace std;

/// @brief Класс для обработки текста
class TextProcessor {
private:
    string text; // Исходный текст для последующей обработки
    vector<string> sentences; // Массив извлечённых предложений из текста
    const string delimiters = ".!?"; // Разделители предложений

public:
    /// @brief Конструктор по умолчанию. Создаёт объект с пустыми полями
    TextProcessor() = default;

    /// @brief Конструктор с параметрами. Принимает строку текста и сохраняет её в виде предложений
    /// @param t строка текста
    TextProcessor(const string& t) : text(t) {
        splitIntoSentences();
    }

    /// @brief Загрузка текста из файла
    /// @param filename название файла
    void loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "ERROR: Cannot open file " << filename << endl;
            return;
        }

        text.clear();
        string line;
        while (getline(file, line)) {
            text += line + " ";
        }
        file.close();

        // Убираем лишние пробелы в начале и конце
        text.erase(0, text.find_first_not_of(" \t\n\r"));
        text.erase(text.find_last_not_of(" \t\n\r") + 1);

        cout << "Text loaded from " << filename << endl;
        splitIntoSentences();
    }

    /// @brief Сохранение текста в файл
    /// @param filename название файла
    void saveToFile(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "ERROR: Cannot open file " << filename << endl;
            return;
        }

        file << text;
        file.close();
        cout << "Text saved to " << filename << endl;
    }

    /// @brief Сеттер для текста в процессоре. Принимает строку и разбивает её на предложения
    /// @param t новый текст для добавления в процессор
    void setText(const string& t) {
        text = t;
        splitIntoSentences();
    }

    /// @brief Геттер для текста в процессоре
    /// @return указатель на строку с текстом
    const string& getText() const { return text; }

    /// @brief Геттер для предложений в процессоре
    /// @return указатель на массив предложений
    const vector<string>& getSentences() const { return sentences; }

    /// @brief Поиск и вывод самых частых символов в каждом предложении в процессоре
    void findMostFrequentLetterInEachSentence() const {
        if (sentences.empty()) {
            cout << "ERROR: No sentences found in the text." << endl;
            return;
        }
        // Для каждого предложения получаем набор символов и находим самый частый из них
        for (size_t i = 0; i < sentences.size(); ++i) { 
            string letters = extractLettersOnly(sentences[i]);
            auto [letter, count] = findMostFrequentLetter(letters);

            if (letter != '\0') {
                cout << "Sentence #" << (i + 1) << ": " << static_cast<char>(toupper(letter)) << " " << count << "." << endl;
            } else {
                cout << "Sentence #" << (i + 1) << ": No letters found." << endl;
            }
        }
    }

    /// @brief Разделение текста в процессоре на предложения по символам-разделителям
    void splitIntoSentences() {
        sentences.clear();
        string current_sentence; // Текущее обрабатываемое предложение
        for (char c : text) {
            if (delimiters.find(c) != string::npos) { // Символ строки является разделителем?
                if (!current_sentence.empty()) { // Предложение закончилось, добавляем в массив
                    sentences.push_back(current_sentence);
                    current_sentence.clear();
                }
            } else { // Символ строки не является разделителем?
                current_sentence += c; // Добавляем его к текущему предложению
            }
        }
        // Добавляем последнее предложение, если оно не пустое
        if (!current_sentence.empty()) {
            sentences.push_back(current_sentence);
        }
    }

    /// @brief Извлечение из строки только букв, игнорируя другие символы
    /// @param sentence строка текста
    /// @return строка, содержащая извлечённые буквы
    string extractLettersOnly(const string& sentence) const {
        string letters; // Строка для хранения букв
        for (char c : sentence) { // Проходимся по каждому символу в строке
            if (isalpha(c)) { // Является ли латинской буквой?
                letters += tolower(c); // Преобразуем в нижний регистр и плюсуем в строку с буквами
            }
        }
        return letters;
    }

    /// @brief Поиск самого частого элемента в строке из букв
    /// @param letters строка с буквами
    /// @return кортеж: буква + количество её вхождений
    pair<char, int> findMostFrequentLetter(const string& letters) const {
        if (letters.empty()) {
            return {'\0', 0};
        }

        map<char, int> freq; // Ассоциативный список (буква: кол-во вхождений)
        for (char c : letters) { // Проход по каждой букве в строке
            freq[c]++; // Увеличение счётчика для отдельной буквы
        }

        int max_freq = 0; // Максимальная частота символа
        for (const auto& p : freq) { // Проход по каждой позиции ассоц. списка
            if (p.second > max_freq) { // Ищем символ с максимумом вхождений
                max_freq = p.second;
            }
        }

        char most_frequent_letter = 'z' + 1; // Самая большая латинская строчная буква
        for (const auto& p : freq) {
            if (p.second == max_freq && p.first < most_frequent_letter) { // Ищем наименьшую букву
                most_frequent_letter = p.first;
            }
        }

        return {most_frequent_letter, max_freq};
    }

    /// @brief Вывод текста их процессора
    void displayText() const {
        cout << "\n--- Original Text ---" << endl;
        cout << text << endl;
    }

    /// @brief Вывод массива предложений из процессора
    void displaySentences() const {
        cout << "\n--- Sentences ---" << endl;
        for (size_t i = 0; i < sentences.size(); ++i) {
            cout << "#" << (i + 1) << ": " << sentences[i] << endl;
        }
    }
};

#endif