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
    string text; // Исходный текст
    vector<string> sentences; // Массив предложений
    const string delimiters = ".!?"; // Разделители предложений

public:
    TextProcessor() = default;
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

    /// @brief Поиск и вывод самых частых символов в каждом предложении
    void findMostFrequentLetterInEachSentence() const {
        if (sentences.empty()) {
            cout << "ERROR: No sentences found in the text." << endl;
            return;
        }

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

    // Вспомогательные методы
    void splitIntoSentences() {
        sentences.clear();
        string current_sentence;
        for (char c : text) {
            if (delimiters.find(c) != string::npos) {
                if (!current_sentence.empty()) {
                    sentences.push_back(current_sentence);
                    current_sentence.clear();
                }
            } else {
                current_sentence += c;
            }
        }
        // Добавляем последнее предложение, если оно не пустое
        if (!current_sentence.empty()) {
            sentences.push_back(current_sentence);
        }
    }

    string extractLettersOnly(const string& sentence) const {
        string letters;
        for (char c : sentence) {
            if (isalpha(c)) {
                letters += tolower(c);
            }
        }
        return letters;
    }

    /// @brief Поиск самого частого элемента в строке
    /// @param letters строка с текстом
    /// @return кортеж из символа и количества его появлений в строке
    pair<char, int> findMostFrequentLetter(const string& letters) const {
        if (letters.empty()) {
            return {'\0', 0};
        }

        map<char, int> freq; // Словарь для подсчёта частот
        for (char c : letters) {
            freq[c]++;
        }

        int max_freq = 0; // Максимальная частота символа
        for (const auto& p : freq) {
            if (p.second > max_freq) {
                max_freq = p.second;
            }
        }

        // Ищем букву с максимальной частотой, которая идёт первой по алфавиту
        char most_frequent_letter = 'z' + 1; // начальное значение для сравнения
        for (const auto& p : freq) {
            if (p.second == max_freq && p.first < most_frequent_letter) {
                most_frequent_letter = p.first;
            }
        }

        return {most_frequent_letter, max_freq};
    }

    // Методы для меню
    void displayText() const {
        cout << "\n--- Original Text ---" << endl;
        cout << text << endl;
    }

    void displaySentences() const {
        cout << "\n--- Sentences ---" << endl;
        for (size_t i = 0; i < sentences.size(); ++i) {
            cout << "#" << (i + 1) << ": " << sentences[i] << endl;
        }
    }
};

#endif // TEXTPROCESSOR_H