#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;

// Шаблон узла дерева
template <typename T>
struct Node {
    T key; // Значение ключа
    Node* left; // Указатель на левое поддерево
    Node* right; // Указатель на правое поддерево
    int height; // Высота узла

    // Конструктор узла
    Node(T k) : key(k), left(nullptr), right(nullptr), height(1) {}
};

// Шаблон класса бинарного дерева поиска
template <typename T>
class Tree {
protected:
    Node<T>* root; // Корень дерева

    // Рекурсивное копирование дерева
    Node<T>* copy(Node<T>* node) {
        if (!node)
            return nullptr;
        Node<T>* newNode = new Node<T>(node->key);
        newNode->left = copy(node->left);
        newNode->right = copy(node->right);
        newNode->height = node->height;
        return newNode;
    }

    // Очистка дерева
    void clear(Node<T>* node) {
        if (!node)
            return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    // Добавление узла
    virtual Node<T>* add(Node<T>* node, T key) {
        if (!node)
            return new Node<T>(key);
        if (key < node->key)
            node->left = add(node->left, key);
        else
            node->right = add(node->right, key);
        return node;
    }

    // Поиск узла по ключу
    Node<T>* search(Node<T>* node, T key) const {
        if (!node)
            return nullptr;
        if (node->key == key)
            return node;
        if (key < node->key)
            return search(node->left, key);
        else
            return search(node->right, key);
    }

    // Поиск минимального узла
    Node<T>* findMin(Node<T>* node) {
        while (node && node->left)
            node = node->left;
        return node;
    }

    // Удаление узла по ключу
    Node<T>* remove(Node<T>* node, T key) {
        if (!node)
            return nullptr;
        if (key < node->key)
            node->left = remove(node->left, key);
        else if (key > node->key)
            node->right = remove(node->right, key);
        else { // Нужный узел найден. 
            if (!node->left) {
                Node<T>* temp = node->right;
                delete node;
                return temp;
            }
            else if (!node->right) {
                Node<T>* temp = node->left;
                delete node;
                return temp;
            }
            // Узел имеет двух потомков. Находим минимальный элемент в правом поддереве.
            Node<T>* temp = findMin(node->right);
            node->key = temp->key; // Копируем значение
            node->right = remove(node->right, temp->key); // Удаляем дубликат
        }
        return node;
    }

    // Обходы дерева

    // Прямой (префиксный) обход дерева
    void pre(Node<T>* node) const {
        if (!node)
            return;
        cout << node->key << " ";
        pre(node->left);
        pre(node->right);
    }

    // Обратный (инфиксный) обход дерева.
    void in(Node<T>* node) const {
        if (!node)
            return;
        in(node->left);
        cout << node->key << " ";
        in(node->right);
    }

    // Концевой (постфиксный) обход дерева.
    void post(Node<T>* node) const {
        if (!node)
            return;
        post(node->left);
        post(node->right);
        cout << node->key << " ";
    }

    // Вывод дерева в поток
    void print(Node<T>* node, int level, ostream& os, T parentKey = T{}) const {
        if (!node)
            return;

        // Печатаем текущий узел с указанием уровня и родителя
        if (parentKey == T{}) {
            os << "Root: " << node->key << endl;
        }
        else {
            os << "node: " << node->key << " (Parent: " << parentKey << ")" << endl;
        }

        // Рекурсивно печатаем левое поддерево
        if (node->left) {
            os << string((level + 1) * 2, ' ') << "Left ";
            print(node->left, level + 1, os, node->key);
        }

        // Рекурсивно печатаем правое поддерево
        if (node->right) {
            os << string((level + 1) * 2, ' ') << "Right ";
            print(node->right, level + 1, os, node->key);
        }
    }

    // Сбор всех значений узлов в вектор
    void collectValues(Node<T>* node, vector<T>& vals) {
        if (!node)
            return;
        vals.push_back(node->key);
        collectValues(node->left, vals);
        collectValues(node->right, vals);
    }

    // Сбор узлов, находящихся на заданном уровне, в вектор
    void collectAtLevel(Node<T>* node, int targetLevel, int currentLevel, vector<T>& result) {
        if (!node)
            return;
        if (currentLevel == targetLevel) {
            result.push_back(node->key);
        }
        collectAtLevel(node->left, targetLevel, currentLevel + 1, result);
        collectAtLevel(node->right, targetLevel, currentLevel + 1, result);
    }

    // Сравнение структуры и данных двух деревьев
    bool compare(Node<T>* a, Node<T>* b) const {
        if (!a && !b)
            return true;
        if (!a || !b)
            return false;
        return (a->key == b->key) &&
               compare(a->left, b->left) &&
               compare(a->right, b->right);
    }

    // Сохранение дерева в файл (префиксный порядок)
    void saveHelper(Node<T>* node, ofstream& out) {
        if (!node)
            return;
        out << node->key << " ";
        saveHelper(node->left, out);
        saveHelper(node->right, out);
    }

public:
    // Конструктор по умолчанию
    Tree() : root(nullptr) {}

    // Конструктор копирования
    Tree(const Tree& other) {
        root = copy(other.root);
    }

    // Виртуальный деструктор
    virtual ~Tree() {
        clear(root);
    }

    // Оператор присваивания
    Tree& operator=(const Tree& other) {
        if (this != &other) {
            clear(root);
            root = copy(other.root);
        }
        return *this;
    }

    // Оператор сравнения
    bool operator==(const Tree& other) const {
        return compare(root, other.root);
    }

    // Оператор вывода
    friend ostream& operator<<(ostream& os, const Tree<T>& tree) {
        tree.print(tree.root, 0, os);
        return os;
    }

    // Оператор ввода
    friend istream& operator>>(istream& is, Tree<T>& tree) {
        T val;
        cout << "Enter value to add: ";
        if (is >> val) {
            tree.addValue(val);
        }
        return is;
    }

    // Основные методы
    void addValue(T key) {
        root = add(root, key);
    }
    void removeValue(T key) {
        root = remove(root, key);
    }
    bool searchValue(T key) const {
        return search(root, key) != nullptr;
    }

    // Визуальный вывод дерева
    void printTree() const {
        print(root, 0, cout);
    }

    // Обходы
    void preOrder() const { pre(root); cout << endl; }
    void inOrder() const { in(root); cout << endl; }
    void postOrder() const { post(root); cout << endl; }

    // Загрузка в файл
    void saveToFile(const string& filename) {
        ofstream out(filename);
        if (!out.is_open()) {
            cout << "Error opening file for writing.\n";
            return;
        }
        saveHelper(root, out);
        out.close();
        cout << "Tree saved to file " << filename << endl;
    }

    // Загрузка из файла
    void loadFromFile(const string& filename) {
        ifstream in(filename);
        if (!in.is_open()) {
            cout << "Error opening file for reading.\n";
            return;
        }
        T val;
        clear(root);
        root = nullptr;
        while (in >> val) {
            addValue(val);
        }
        in.close();
        cout << "Tree loaded from file " << filename << endl;
    }

    // ЗАДАНИЕ 5 ВАРИАНТА
    // Выводит значений узлов с заданного уровня
    void executeVariantTask() {
        if (!root) {
            cout << "Tree is empty.\n";
            return;
        }

        // Запрашиваем у пользователя уровень
        int targetLevel;
        cout << "Enter the level to display nodes from: ";
        cin >> targetLevel;

        // Проверяем корректность ввода
        if (targetLevel < 1) {
            cout << "Level must be >= 1.\n";
            return;
        }

        // Собираем все значения узлов, находящихся на заданном уровне
        vector<T> nodesAtLevel;
        collectAtLevel(root, targetLevel, 1, nodesAtLevel);

        // Выводим результат
        cout << "\n--- Nodes at Level " << targetLevel << " ---\n";
        if (nodesAtLevel.empty()) {
            cout << "No nodes found at this level.\n";
        }
        else {
            for (const auto& val : nodesAtLevel) {
                cout << val << " ";
            }
            cout << endl;
        }
    }
};

// Сбалансированное бинарное дерево (AVL)
template <typename T>
class AVL : public Tree<T> {
    using Tree<T>::root; // Доступ к root из базового класса

    // Вспомогательные функции для AVL
    int height(Node<T>* node) {
        return node ? node->height : 0; }
    int balance(Node<T>* node) {
        return node ? height(node->left) - height(node->right) : 0; }

    // Правый поворот
    Node<T>* rotateRight(Node<T>* y) {
        Node<T>* x = y->left;
        Node<T>* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;
        return x;
    }

    // Левый поворот
    Node<T>* rotateLeft(Node<T>* x) {
        Node<T>* y = x->right;
        Node<T>* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;
        return y;
    }

    // Рекурсивная вставка с балансировкой
    Node<T>* addAVL(Node<T>* node, T key) {
        // Стандартная вставка BST
        if (!node)
            return new Node<T>(key);
        if (key < node->key) node->left = addAVL(node->left, key);
        else node->right = addAVL(node->right, key);

        // Обновление высоты
        node->height = max(height(node->left), height(node->right)) + 1;

        // Проверка баланса
        int bf = balance(node);

        // Случаи вращения
        // Left Left
        if (bf > 1 && key < node->left->key) 
            return rotateRight(node);
        // Right Right
        if (bf < -1 && key > node->right->key) 
            return rotateLeft(node);
        // Left Right
        if (bf > 1 && key > node->left->key) { 
            node->left = rotateLeft(node->left); 
            return rotateRight(node);
        }
        // Right Left
        if (bf < -1 && key < node->right->key) { 
            node->right = rotateRight(node->right); 
            return rotateLeft(node);
        }

        return node;
    }

protected:
    // Переопределение виртуального метода добавления
    Node<T>* add(Node<T>* node, T key) override {
        return addAVL(node, key);
    }
};

#endif