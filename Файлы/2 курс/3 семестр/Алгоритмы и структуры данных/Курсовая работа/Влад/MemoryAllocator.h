#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <cstddef>
#include <iostream>

/**
 * @brief Структура заголовка блока памяти.
 * 
 * Каждый блок (занятый или свободный) в эмулируемой куче начинается
 * с этого заголовка. Содержит информацию для взаимодействия с блоком.
 */
struct BlockHeader {
    size_t size; // Общий размер блока в байтах, включая сам заголовок
    bool isAllocated; // Флаг занятости: `true` — блок используется, `false` — свободен
};

/**
 * @brief Класс программной модели управления оперативной памятью.
 * 
 * Эмулирует работу аллокатора кучи на фиксированном буфере. Поддерживает:
 * - выделение памяти по стратегии first-fit,
 * - освобождение блоков,
 * - автоматическое слияние (coalescing) соседних свободных блоков,
 * - расчёт степени внешней фрагментации.
 */
class MemoryAllocator {
public:
    /**
     * @brief Конструктор аллокатора.
     * @param heapSize Размер эмулируемой кучи в байтах (по умолчанию 64 КБ = 65536 байт).
     */
    explicit MemoryAllocator(size_t heapSize = 65536);

    /**
     * @brief Деструктор.
     */
    ~MemoryAllocator();

    /**
     * @brief Выделяет блок памяти заданного размера.
     * @param userSize Требуемый размер данных в байтах (должен быть > 0).
     * @return Указатель на начало данных или `nullptr`, если память не может быть выделена.
     */
    void* allocate(size_t userSize);

    /**
     * @brief Освобождает ранее выделенный блок памяти.
     * @param ptr Указатель, ранее полученный от метода `allocate()`.
     */
    void deallocate(void* ptr);

    /**
     * @brief Выводит детальную статистику о состоянии эмулируемой кучи.
     */
    void printStatus() const;

    /**
     * @brief Возвращает текущую степень внешней фрагментации.
     * @return Значение от 0.0 (полное отсутствие фрагментации)
     *         до 1.0 (максимальная фрагментация).
     */
    double getFragmentation() const;

    /**
     * @brief Преобразует указатель на данные в указатель на заголовок блока.
     * @param ptr Указатель, возвращённый `allocate()`, должен быть валидным.
     * @return Указатель на структуру `BlockHeader`, предшествующую данным.
     */
    BlockHeader* headerFromPtr(void* ptr) const;

    /**
     * @brief Проверяет, что указатель находится в пределах эмулируемой кучи.
     * @param ptr Проверяемый адрес.
     * @return `true` или `false` в зависимости от валидности адреса.
     */
    bool isValidPtr(void* ptr) const;

    /**
     * @brief Преобразует указатель на заголовок в указатель на данные.
     * @param header Указатель на заголовок блока.
     * @return Указатель на начало пользовательских данных (header + HEADER_SIZE).
     */
    void* ptrFromHeader(BlockHeader* header) const;

private:
    /// Выравнивание памяти по 8-байтной границе
    static constexpr size_t ALIGNMENT = 8;

    /// Размер заголовка блока (в байтах)
    static constexpr size_t HEADER_SIZE = sizeof(BlockHeader);

    /// Указатель на начало эмулируемой кучи (буфера в реальной памяти)
    char* heapStart;

    /// Общий размер эмулируемой кучи в байтах
    size_t heapSize;

    /**
     * @brief Округляет значение вверх до ближайшего кратного `alignment`.
     * @param value Исходное значение.
     * @param alignment Требуемое выравнивание (должно быть степенью двойки).
     * @return Выровненное значение.
     */
    size_t alignUp(size_t value, size_t alignment) const;

    /**
     * @brief Выполняет полное слияние соседних свободных блоков.
     */
    void coalesceHeap();
};

#endif