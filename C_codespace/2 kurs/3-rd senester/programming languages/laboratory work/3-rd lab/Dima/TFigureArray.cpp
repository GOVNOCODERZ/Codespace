#include "TFigureArray.h"
#include "Square.h"
#include "Circle.h"

#include <stdexcept>
#include <vector>

// --- Реализация приватных методов ---

template <typename T>
void TFigureArray<T>::check_index(int index) const {
    if (index < 0 || index >= size) {
        throw std::out_of_range("Index out of range");
    }
}

template <typename T>
void TFigureArray<T>::resize(int new_capacity) {
    if (new_capacity <= size) {
        return;
    }
    T* new_data = new T[new_capacity];
    for (int i = 0; i < size; ++i) {
        new_data[i] = data[i];
    }
    delete[] data;
    data = new_data;
    capacity = new_capacity;
}

// --- Конструкторы и деструктор ---

template <typename T>
TFigureArray<T>::TFigureArray() : data(nullptr), size(0), capacity(0) {}

template <typename T>
TFigureArray<T>::TFigureArray(const TFigureArray<T>& other) : size(other.size), capacity(other.capacity) {
    data = new T[capacity];
    for (int i = 0; i < size; ++i) {
        data[i] = other.data[i];
    }
}

template <typename T>
TFigureArray<T>::~TFigureArray() {
    delete[] data;
}

// --- Операторы ---

template <typename T>
TFigureArray<T>& TFigureArray<T>::operator=(const TFigureArray<T>& other) {
    if (this == &other) {
        return *this;
    }
    delete[] data;
    size = other.size;
    capacity = other.capacity;
    data = new T[capacity];
    for (int i = 0; i < size; ++i) {
        data[i] = other.data[i];
    }
    return *this;
}

template <typename T>
bool TFigureArray<T>::operator==(const TFigureArray<T>& other) const {
    if (size != other.size) return false;
    for (int i = 0; i < size; ++i) {
        if (!(data[i] == other.data[i])) { // Требует operator== для типа T
            return false;
        }
    }
    return true;
}

template <typename T>
bool TFigureArray<T>::operator!=(const TFigureArray<T>& other) const {
    return !(*this == other);
}


template <typename T>
T& TFigureArray<T>::operator[](int index) {
    check_index(index);
    return data[index];
}

template <typename T>
const T& TFigureArray<T>::operator[](int index) const {
    check_index(index);
    return data[index];
}

// --- Публичные методы ---

template <typename T>
int TFigureArray<T>::GetSize() const {
    return size;
}

template <typename T>
void TFigureArray<T>::AddElement(const T& element) {
    if (size >= capacity) {
        resize(capacity == 0 ? 1 : capacity * 2);
    }
    data[size++] = element;
}

template<typename T>
void TFigureArray<T>::ModifyElementFromConsole(int index) {
    check_index(index);
    std::cout << "Enter new data for element at index " << index << ": ";
    T new_value;
    std::cin >> new_value;
    data[index] = new_value;
}

template <typename T>
void TFigureArray<T>::SetRandomValue(int index) {
    check_index(index);
    data[index].randomize(); // Требует наличия метода randomize() у типа T
}

template <typename T>
void TFigureArray<T>::GenerateRandomValues(int count) {
    if (count < 0) return;
    delete[] data;
    size = count;
    capacity = count;
    data = new T[capacity];
    for (int i = 0; i < size; ++i) {
        data[i].randomize();
    }
}

// --- Методы, специфичные для лабораторной работы ---

template <typename T>
void TFigureArray<T>::ShellSortByArea() {
    for (int gap = size / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < size; ++i) {
            T temp = data[i];
            int j;
            for (j = i; j >= gap && data[j - gap] > temp; j -= gap) { // Требует operator> для типа T
                data[j] = data[j - gap];
            }
            data[j] = temp;
        }
    }
}

template <typename T>
std::vector<T> TFigureArray<T>::FindFiguresWithAreaGreaterThanAverage() const {
    if (size == 0) {
        return std::vector<T>();
    }
    double total_area = 0.0;
    for (int i = 0; i < size; ++i) {
        total_area += data[i].getArea(); // Требует наличия метода getArea() у типа T
    }
    double average_area = total_area / size;
    std::cout << "\nAverage area: " << average_area << "\n";
    std::vector<T> result;
    for (int i = 0; i < size; ++i) {
        if (data[i].getArea() > average_area) {
            result.push_back(data[i]);
        }
    }
    return result;
}

// --- Реализация дружественных операторов ---

template <typename U>
std::ostream& operator<<(std::ostream& os, const TFigureArray<U>& arr) {
    os << "Array (size = " << arr.GetSize() << "):\n";
    for (int i = 0; i < arr.GetSize(); ++i) {
        os << "[" << i << "] " << arr[i] << "\n";
    }
    return os;
}

template <typename U>
std::istream& operator>>(std::istream& is, TFigureArray<U>& arr) {
    int new_size;
    is >> new_size;
    if (!is || new_size < 0) return is;
    
    arr.GenerateRandomValues(0); // Очистка массива

    for (int i = 0; i < new_size; ++i) {
        U element;
        is >> element;
        if (is) {
            arr.AddElement(element);
        } else {
            break; 
        }
    }
    return is;
}

// --- Явное инстанцирование шаблонов ---
// Это позволяет компилятору сгенерировать код для указанных типов,
// несмотря на то, что реализация находится в .cpp файле.

template class TFigureArray<Square>;
template std::ostream& operator<<(std::ostream& os, const TFigureArray<Square>& arr);
template std::istream& operator>>(std::istream& is, TFigureArray<Square>& arr);

template class TFigureArray<Circle>;
template std::ostream& operator<<(std::ostream& os, const TFigureArray<Circle>& arr);
template std::istream& operator>>(std::istream& is, TFigureArray<Circle>& arr);