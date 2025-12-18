#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm> // для max
#include <cmath>     // для мат. операций

using namespace std;

// Шаблон узла дерева
template <typename T>
struct Node {
    T key;
    Node* left;
    Node* right;
    int height; // для балансировки

    Node(T k) : key(k), left(nullptr), right(nullptr), height(1) {}
};

// Шаблон класса Бинарное дерево поиска
template <typename T>
class Tree {
protected:
    Node<T>* root;

    // Рекурсивное копирование
    Node<T>* copy(Node<T>* node) {
        if (!node) return nullptr;
        Node<T>* newNode = new Node<T>(node->key);
        newNode->left = copy(node->left);
        newNode->right = copy(node->right);
        newNode->height = node->height;
        return newNode;
    }

    // Рекурсивная очистка
    void clear(Node<T>* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    // Виртуальная функция добавления (переопределяется в AVL)
    virtual Node<T>* add(Node<T>* node, T key) {
        if (!node) return new Node<T>(key);
        if (key < node->key) node->left = add(node->left, key);
        else node->right = add(node->right, key);
        return node;
    }

    // Поиск узла
    Node<T>* search(Node<T>* node, T key) const {
        if (!node) return nullptr;
        if (node->key == key) return node;
        if (key < node->key) return search(node->left, key);
        else return search(node->right, key);
    }

    // Поиск минимума (для удаления)
    Node<T>* findMin(Node<T>* node) {
        while (node && node->left) node = node->left;
        return node;
    }

    // Удаление узла
    Node<T>* remove(Node<T>* node, T key) {
        if (!node) return nullptr;
        if (key < node->key) node->left = remove(node->left, key);
        else if (key > node->key) node->right = remove(node->right, key);
        else {
            if (!node->left) {
                Node<T>* temp = node->right;
                delete node;
                return temp;
            } else if (!node->right) {
                Node<T>* temp = node->left;
                delete node;
                return temp;
            }
            Node<T>* temp = findMin(node->right);
            node->key = temp->key;
            node->right = remove(node->right, temp->key);
        }
        return node;
    }

    // Обходы дерева
    void pre(Node<T>* node) const {
        if (!node) return;
        cout << node->key << " ";
        pre(node->left);
        pre(node->right);
    }

    void in(Node<T>* node) const {
        if (!node) return;
        in(node->left);
        cout << node->key << " ";
        in(node->right);
    }

    void post(Node<T>* node) const {
        if (!node) return;
        post(node->left);
        post(node->right);
        cout << node->key << " ";
    }

    // Вывод дерева в поток (для operator<<)
    void print(Node<T>* node, int indent, ostream& os) const {
        if (!node) return;
        if (node->right) print(node->right, indent + 4, os);
        if (indent) os << setw(indent) << ' ';
        os << node->key << "\n";
        if (node->left) print(node->left, indent + 4, os);
    }

    // Вспомогательные функции для Варианта 13
    void collectValues(Node<T>* node, vector<T>& vals) {
        if (!node) return;
        vals.push_back(node->key);
        collectValues(node->left, vals);
        collectValues(node->right, vals);
    }

    void collectGreater(Node<T>* node, double avg, int level, vector<pair<T,int>>& result) {
        if (!node) return;
        if (static_cast<double>(node->key) > avg) result.push_back({node->key, level});
        collectGreater(node->left, avg, level + 1, result);
        collectGreater(node->right, avg, level + 1, result);
    }

    // Сравнение структуры и данных
    bool compare(Node<T>* a, Node<T>* b) const {
        if (!a && !b) return true;
        if (!a || !b) return false;
        return (a->key == b->key) &&
               compare(a->left, b->left) &&
               compare(a->right, b->right);
    }

    void saveHelper(Node<T>* node, ofstream& out, int level) {
        if (!node) return;
        // Простой формат для сохранения: Значение (можно улучшить структуру, 
        // но для простого считывания достаточно pre-order)
        out << node->key << " "; 
        saveHelper(node->left, out, level+1);
        saveHelper(node->right, out, level+1);
    }

public:
    Tree() : root(nullptr) {}
    
    // Конструктор копирования
    Tree(const Tree& other) { root = copy(other.root); }
    
    // Виртуальный деструктор
    virtual ~Tree() { clear(root); }

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

    // Оператор вывода (дружественная функция)
    friend ostream& operator<<(ostream& os, const Tree<T>& tree) {
        tree.print(tree.root, 0, os);
        return os;
    }

    // Оператор ввода (дружественная функция)
    friend istream& operator>>(istream& is, Tree<T>& tree) {
        T val;
        cout << "Введите значение для добавления: ";
        if (is >> val) {
            tree.addValue(val);
        }
        return is;
    }

    // Основные методы
    void addValue(T key) { root = add(root, key); }
    void removeValue(T key) { root = remove(root, key); }
    bool searchValue(T key) const { return search(root, key) != nullptr; }
    
    void printTree() const { print(root, 0, cout); } // Для совместимости

    void preOrder() const { pre(root); cout << endl; }
    void inOrder() const { in(root); cout << endl; }
    void postOrder() const { post(root); cout << endl; }

    // Работа с файлами
    void saveToFile(const string& filename) {
        ofstream out(filename);
        if (!out.is_open()) {
            cout << "Ошибка открытия файла для записи.\n";
            return;
        }
        // Сохраняем последовательность значений (pre-order удобен для восстановления)
        saveHelper(root, out, 0);
        out.close();
        cout << "Дерево сохранено в файл " << filename << endl;
    }

    void loadFromFile(const string& filename) {
        ifstream in(filename);
        if (!in.is_open()) {
            cout << "Ошибка открытия файла для чтения.\n";
            return;
        }
        T val;
        clear(root);
        root = nullptr;
        while (in >> val) {
            addValue(val);
        }
        in.close();
        cout << "Дерево загружено из файла " << filename << endl;
    }

    // ЗАДАНИЕ ВАРИАНТА 13
    void executeVariantTask() {
        if (!root) {
            cout << "Дерево пусто.\n";
            return;
        }
        vector<T> vals;
        collectValues(root, vals);
        
        double sum = 0;
        for (T v : vals) sum += static_cast<double>(v);
        double avg = sum / vals.size();

        vector<pair<T,int>> result;
        collectGreater(root, avg, 1, result);

        cout << fixed << setprecision(3);
        cout << "Результат\n";
        cout << "Среднее арифметическое: " << avg << "\n";
        cout << "Узлы со значением больше среднего:\n";
        if (result.empty()) {
            cout << "Таких узлов нет.\n";
        } else {
            cout << "Значение | Уровень\n";
            for (const auto& p : result) {
                cout << setw(8) << p.first << " | " << p.second << "\n";
            }
        }
    }
};

// Сбалансированное бинарное дерево (AVL)
template <typename T>
class AVL : public Tree<T> {
    using Tree<T>::root; // Доступ к root из базового класса

    int height(Node<T>* node) { return node ? node->height : 0; }
    int balance(Node<T>* node) { return node ? height(node->left) - height(node->right) : 0; }

    Node<T>* rotateRight(Node<T>* y) {
        Node<T>* x = y->left;
        Node<T>* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;
        return x;
    }

    Node<T>* rotateLeft(Node<T>* x) {
        Node<T>* y = x->right;
        Node<T>* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;
        return y;
    }

    Node<T>* addAVL(Node<T>* node, T key) {
        // Стандартная вставка BST
        if (!node) return new Node<T>(key);
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

// Функция для вывода меню
void showMenu() {
    cout << "\n============== МЕНЮ ==============\n";
    cout << "1. Добавить узел (ввод с клавиатуры)\n";
    cout << "2. Удалить узел\n";
    cout << "3. Найти узел\n";
    cout << "4. Вывести дерево (визуально)\n";
    cout << "5. Обходы дерева (Прямой, Обратный, Концевой)\n";
    cout << "6. Среднее и фильтр\n";
    cout << "7. Сохранить в файл\n";
    cout << "8. Загрузить из файла\n";
    cout << "9. Тест операторов (= и ==)\n";
    cout << "10. Использовать operator>> (ввод одного значения)\n";
    cout << "0. Выход\n";
    cout << "==================================\n";
    cout << "Ваш выбор: ";
}

int main() {
    // Используем AVL<double> согласно варианту с вещественными числами
    AVL<double> tree;
    int choice;
    double val;
    string filename;

    // Для теста операторов
    AVL<double> testTree; 

    while (true) {
        showMenu();
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
        case 1:
            cout << "Введите значение (double): ";
            cin >> val;
            tree.addValue(val);
            cout << "Узел добавлен.\n";
            break;
        case 2:
            cout << "Введите значение для удаления: ";
            cin >> val;
            tree.removeValue(val);
            cout << "Команда удаления выполнена (если узел существовал).\n";
            break;
        case 3:
            cout << "Введите значение для поиска: ";
            cin >> val;
            if (tree.searchValue(val)) cout << "Узел НАЙДЕН.\n";
            else cout << "Узел НЕ найден.\n";
            break;
        case 4:
            cout << "\n--- Текущее дерево (operator<<) ---\n";
            cout << tree; // Использование перегруженного оператора <<
            break;
        case 5:
            cout << "Прямой обход (PreOrder): "; tree.preOrder();
            cout << "Обратный обход (InOrder): "; tree.inOrder();
            cout << "Концевой обход (PostOrder): "; tree.postOrder();
            break;
        case 6:
            tree.executeVariantTask();
            break;
        case 7:
            cout << "Введите имя файла: ";
            cin >> filename;
            tree.saveToFile(filename);
            break;
        case 8:
            cout << "Введите имя файла: ";
            cin >> filename;
            tree.loadFromFile(filename);
            break;
        case 9:
            cout << "\n--- Тестирование операторов ---\n";
            testTree = tree; // Тест operator=
            cout << "Дерево скопировано в testTree через operator=.\n";
            cout << "Сравнение tree == testTree: " << (tree == testTree ? "РАВНЫ" : "НЕ РАВНЫ") << endl;
            
            cout << "Добавим 999.9 в testTree...\n";
            testTree.addValue(999.9);
            cout << "Сравнение tree == testTree: " << (tree == testTree ? "РАВНЫ" : "НЕ РАВНЫ") << endl;
            break;
        case 10:
            // Тест operator>>
            cin >> tree;
            break;
        case 0:
            return 0;
        default:
            cout << "Неверный выбор. Попробуйте снова.\n";
        }
    }
}