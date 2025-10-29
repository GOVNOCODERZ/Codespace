#ifndef TFIGUREARRAY_H
#define TFIGUREARRAY_H

#include <iostream>
#include <vector>

template <typename T>
class TFigureArray {
private:
    T* data;      // Указатель на динамический массив
    int size;     // Текущее количество элементов
    int capacity; // Выделенная память

    // Вспомогательный метод для проверки корректности индекса
    void check_index(int index) const;
    // Приватный метод для изменения размера массива
    void resize(int new_capacity);

public:
    // Конструкторы и деструктор
    TFigureArray();
    TFigureArray(const TFigureArray<T>& other); // Конструктор копирования
    ~TFigureArray();

    // Операторы
    TFigureArray<T>& operator=(const TFigureArray<T>& other); // Оператор присваивания
    bool operator==(const TFigureArray<T>& other) const;      // Оператор сравнения
    bool operator!=(const TFigureArray<T>& other) const;

    // Доступ к элементам
    T& operator[](int index);
    const T& operator[](int index) const;

    // Публичные методы
    int GetSize() const;
    void AddElement(const T& element);
    void ModifyElementFromConsole(int index);
    void SetRandomValue(int index);
    void GenerateRandomValues(int count);

    // Методы, специфичные для лабораторной работы
    void ShellSortByArea();
    std::vector<T> FindFiguresWithAreaGreaterThanAverage() const;
};

// Объявления шаблонных дружественных функций
template <typename U>
std::ostream& operator<<(std::ostream& os, const TFigureArray<U>& arr);
template <typename U>
std::istream& operator>>(std::istream& is, TFigureArray<U>& arr);


#endif // TFIGUREARRAY_H