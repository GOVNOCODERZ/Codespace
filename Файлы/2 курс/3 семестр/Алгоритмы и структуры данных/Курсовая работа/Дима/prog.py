import numpy as np
import pygad
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, OneHotEncoder
import matplotlib.pyplot as plt

# ==========================================
# 1. НАСТРОЙКИ
# ==========================================
NUM_GENERATIONS = 50
SOL_PER_POP = 10
NUM_PARENTS_MATING = 4
MUTATION_PERCENT = 15

# Архитектура сети
INPUT_SIZE = 4      # 4 признака у ирисов
HIDDEN_SIZE = 8     # Скрытый слой
OUTPUT_SIZE = 3     # 3 класса ирисов

# ==========================================
# 2. ДАННЫЕ
# ==========================================
iris = load_iris()
X = iris.data
y = iris.target.reshape(-1, 1)

# One-hot encoding
encoder = OneHotEncoder(sparse_output=False)
y_encoded = encoder.fit_transform(y)

X_train, X_test, y_train, y_test = train_test_split(X, y_encoded, test_size=0.2, random_state=42)
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

# ==========================================
# 3. НЕЙРОСЕТЬ НА NUMPY (Вместо Keras)
# ==========================================
def relu(x):
    return np.maximum(0, x)

def softmax(x):
    e_x = np.exp(x - np.max(x, axis=1, keepdims=True))
    return e_x / np.sum(e_x, axis=1, keepdims=True)

def predict(X, weights_vector):
    """
    Ручной проход нейросети (Forward pass).
    Мы сами "нарезаем" длинный вектор генов на матрицы весов.
    """
    # 1. Извлекаем веса Input -> Hidden
    w1_end = INPUT_SIZE * HIDDEN_SIZE
    W1 = weights_vector[0:w1_end].reshape((INPUT_SIZE, HIDDEN_SIZE))
    
    # 2. Извлекаем смещения (biases) для Hidden
    b1_end = w1_end + HIDDEN_SIZE
    b1 = weights_vector[w1_end:b1_end].reshape((1, HIDDEN_SIZE))
    
    # 3. Извлекаем веса Hidden -> Output
    w2_end = b1_end + (HIDDEN_SIZE * OUTPUT_SIZE)
    W2 = weights_vector[b1_end:w2_end].reshape((HIDDEN_SIZE, OUTPUT_SIZE))
    
    # 4. Извлекаем смещения для Output
    b2_end = w2_end + OUTPUT_SIZE
    b2 = weights_vector[w2_end:b2_end].reshape((1, OUTPUT_SIZE))
    
    # --- Вычисления ---
    # Слой 1
    z1 = np.dot(X, W1) + b1
    a1 = relu(z1) # Функция активации
    
    # Слой 2 (Выход)
    z2 = np.dot(a1, W2) + b2
    a2 = softmax(z2) # Вероятности
    
    return a2

# ==========================================
# 4. ГЕНЕТИЧЕСКИЙ АЛГОРИТМ
# ==========================================
def fitness_func(ga_instance, solution, solution_idx):
    # Делаем предсказание с текущими генами (весами)
    predictions = predict(X_train, solution)
    
    # Считаем точность
    y_pred_classes = np.argmax(predictions, axis=1)
    y_true_classes = np.argmax(y_train, axis=1)
    accuracy = np.mean(y_pred_classes == y_true_classes)
    
    return accuracy

def on_generation(ga_instance):
    print(f"Generation = {ga_instance.generations_completed} | "
          f"Accuracy = {ga_instance.best_solution()[1]:.4f}")

# Считаем сколько всего нужно весов (размер хромосомы)
num_genes = (INPUT_SIZE * HIDDEN_SIZE) + HIDDEN_SIZE + (HIDDEN_SIZE * OUTPUT_SIZE) + OUTPUT_SIZE
print(f"Количество параметров для оптимизации: {num_genes}")

ga_instance = pygad.GA(
    num_generations=NUM_GENERATIONS,
    num_parents_mating=NUM_PARENTS_MATING,
    fitness_func=fitness_func,
    sol_per_pop=SOL_PER_POP,
    num_genes=num_genes,
    parent_selection_type="sss",
    keep_parents=2,
    crossover_type="single_point",
    mutation_type="random",
    mutation_percent_genes=MUTATION_PERCENT,
    on_generation=on_generation
)

# Запуск
ga_instance.run()

# ==========================================
# 5. РЕЗУЛЬТАТЫ
# ==========================================
ga_instance.plot_fitness(title="Обучение нейросети (NumPy + PyGAD)", ylabel="Точность")

solution, solution_fitness, _ = ga_instance.best_solution()
print(f"Лучшая точность на обучении: {solution_fitness:.4f}")

# Тест
preds_test = predict(X_test, solution)
acc_test = np.mean(np.argmax(preds_test, axis=1) == np.argmax(y_test, axis=1))
print(f"Точность на тесте: {acc_test*100:.2f}%")
plt.show()