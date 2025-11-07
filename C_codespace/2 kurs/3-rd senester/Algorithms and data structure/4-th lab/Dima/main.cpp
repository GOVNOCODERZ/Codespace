#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
using namespace std;

// Узел дерева
struct Node {
    double key;
    Node* left;
    Node* right;
    int height; // для сбалансированного дерева
    Node(double k) : key(k), left(nullptr), right(nullptr), height(1) {}
};

// Класс бинарного дерева поиска
class Tree {
protected:
    Node* root;

    // Копирование дерева рекурсивно
    Node* copy(Node* node) {
        if (!node) return nullptr;
        Node* newNode = new Node(node->key);
        newNode->left = copy(node->left);
        newNode->right = copy(node->right);
        newNode->height = node->height;
        return newNode;
    }

    // Очистка дерева рекурсивно
    void clear(Node* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    // Вставка узла рекурсивно
    virtual Node* add(Node* node, double key) {
        if (!node) return new Node(key);
        if (key < node->key) node->left = add(node->left, key);
        else node->right = add(node->right, key);
        return node;
    }

    // Поиск узла
    Node* search(Node* node, double key) {
        if (!node) return nullptr;
        if (node->key == key) return node;
        if (key < node->key) return search(node->left, key);
        else return search(node->right, key);
    }

    // Поиск минимального узла
    Node* findMin(Node* node) {
        while (node && node->left) node = node->left;
        return node;
    }

    // Удаление узла
    Node* remove(Node* node, double key) {
        if (!node) return nullptr;
        if (key < node->key) node->left = remove(node->left, key);
        else if (key > node->key) node->right = remove(node->right, key);
        else {
            if (!node->left) {
                Node* temp = node->right;
                delete node;
                return temp;
            } else if (!node->right) {
                Node* temp = node->left;
                delete node;
                return temp;
            }
            Node* temp = findMin(node->right);
            node->key = temp->key;
            node->right = remove(node->right, temp->key);
        }
        return node;
    }

    // Прямой обход
    void pre(Node* node) {
        if (!node) return;
        cout << node->key << " ";
        pre(node->left);
        pre(node->right);
    }

    // Обратный обход
    void in(Node* node) {
        if (!node) return;
        in(node->left);
        cout << node->key << " ";
        in(node->right);
    }

    // Концевой обход
    void post(Node* node) {
        if (!node) return;
        post(node->left);
        post(node->right);
        cout << node->key << " ";
    }

    // Вывод дерева
    void print(Node* node, int indent) {
        if (!node) return;
        if (node->right) print(node->right, indent + 4);
        if (indent) cout << setw(indent) << ' ';
        cout << node->key << "\n";
        if (node->left) print(node->left, indent + 4);
    }

    // Сбор всех значений для среднего
    void collectValues(Node* node, vector<double>& vals) {
        if (!node) return;
        vals.push_back(node->key);
        collectValues(node->left, vals);
        collectValues(node->right, vals);
    }

    // Сбор узлов больше среднего
    void collectGreater(Node* node, double avg, int level, vector<pair<double,int>>& result) {
        if (!node) return;
        if (node->key > avg) result.push_back({node->key, level});
        collectGreater(node->left, avg, level + 1, result);
        collectGreater(node->right, avg, level + 1, result);
    }

    // Вспомогательная функция для сравнения деревьев
    bool compare(Node* a, Node* b) const {
        if (!a && !b) return true;
        if (!a || !b) return false;
        return (a->key == b->key) &&
               compare(a->left, b->left) &&
               compare(a->right, b->right);
    }

public:
    Tree() : root(nullptr) {}
    Tree(const Tree& other) { root = copy(other.root); }
    virtual ~Tree() { clear(root); }

    // Перегрузка оператора присваивания
    Tree& operator=(const Tree& other) {
        if (this != &other) {
            clear(root);
            root = copy(other.root);
        }
        return *this;
    }

    // Перегрузка оператора сравнения
    bool operator==(const Tree& other) const {
        return compare(root, other.root);
    }

    // Интерфейсные функции
    void addValue(double key) { root = add(root, key); }  // публичная обёртка для вставки
    void removeValue(double key) { root = remove(root, key); }
    bool searchValue(double key) { return search(root, key) != nullptr; }
    void printTree() { print(root, 0); }
    void preOrder() { pre(root); cout << endl; }
    void inOrder() { in(root); cout << endl; }
    void postOrder() { post(root); cout << endl; }

    void saveToFile(const string& filename) {
        ofstream out(filename);
        if (!out.is_open()) return;
        saveHelper(root, out, 0);
        out.close();
    }

    void saveHelper(Node* node, ofstream& out, int level) {
        if (!node) return;
        out << string(level*4,' ') << node->key << "\n";
        saveHelper(node->left, out, level+1);
        saveHelper(node->right, out, level+1);
    }

    void loadFromFile(const string& filename) {
        ifstream in(filename);
        double val;
        while (in >> val) addValue(val);
        in.close();
    }

    double average() {
        vector<double> vals;
        collectValues(root, vals);
        if (vals.empty()) return 0;
        double sum = 0;
        for (double v : vals) sum += v;
        return sum / vals.size();
    }

    void showGreaterThanAverage() {
        double avg = average();
        vector<pair<double,int>> result;
        collectGreater(root, avg, 1, result);
        cout << "Среднее арифметическое: " << avg << "\n";
        for (auto& p : result)
            cout << "Значение: " << p.first << ", уровень: " << p.second << "\n";
    }
};

// Сбалансированное бинарное дерево (AVL)
class AVL : public Tree {
    int height(Node* node) { return node ? node->height : 0; }
    int balance(Node* node) { return node ? height(node->left)-height(node->right) : 0; }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = max(height(y->left), height(y->right))+1;
        x->height = max(height(x->left), height(x->right))+1;
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = max(height(x->left), height(x->right))+1;
        y->height = max(height(y->left), height(y->right))+1;
        return y;
    }

    Node* addAVL(Node* node, double key) {
        if (!node) return new Node(key);
        if (key < node->key) node->left = addAVL(node->left, key);
        else node->right = addAVL(node->right, key);

        node->height = max(height(node->left), height(node->right))+1;
        int bf = balance(node);

        if (bf > 1 && key < node->left->key) return rotateRight(node);
        if (bf < -1 && key > node->right->key) return rotateLeft(node);
        if (bf > 1 && key > node->left->key) { node->left = rotateLeft(node->left); return rotateRight(node);}
        if (bf < -1 && key < node->right->key) { node->right = rotateRight(node->right); return rotateLeft(node);}
        return node;
    }

protected:
    Node* add(Node* node, double key) override {
        return addAVL(node, key);
    }
};

int main() {
    AVL tree;
    int n;
    double val;

    cout << "Введите количество элементов: ";
    cin >> n;
    cout << "Введите элементы: ";
    for (int i = 0; i < n; i++) {
        cin >> val;
        tree.addValue(val);
    }

    cout << "\nДерево:\n";
    tree.printTree();

    cout << "\nПрямой обход: ";
    tree.preOrder();
    cout << "Обратный обход: ";
    tree.inOrder();
    cout << "Концевой обход: ";
    tree.postOrder();

    cout << "\nУзлы больше среднего арифметического:\n";
    tree.showGreaterThanAverage();

    return 0;
}
