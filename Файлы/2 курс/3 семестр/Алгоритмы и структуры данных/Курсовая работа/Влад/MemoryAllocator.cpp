#include "MemoryAllocator.h"
#include <cstring>
#include <algorithm>
#include <iostream>
using namespace std;

// === Конструктор и деструктор ===

MemoryAllocator::MemoryAllocator(size_t heapSize)
    : heapSize(heapSize), freeList(nullptr) {
    // Выделяем буфер в реальной куче процесса под эмулируемую кучу
    heapStart = new char[heapSize];

    // Инициализируем первый (и единственный) блок как свободный
    BlockHeader* first = reinterpret_cast<BlockHeader*>(heapStart);
    first->size = heapSize;
    first->isAllocated = false;
    first->nextFree = nullptr;
    freeList = first; // Список свободных блоков теперь указывает на этот блок
}

MemoryAllocator::~MemoryAllocator() {
    // Освобождаем память, выделенную под эмулируемую кучу
    delete[] heapStart;
}

// === Вспомогательные методы ===

size_t MemoryAllocator::alignUp(size_t value, size_t alignment) const {
    // Округление вверх до ближайшего кратного `alignment` с использованием битовой маски
    // Работает корректно только если alignment — степень двойки (например, 8)
    return (value + alignment - 1) & ~(alignment - 1);
}

// === Основные методы аллокатора ===

void* MemoryAllocator::allocate(size_t userSize) {
    if (userSize == 0) return nullptr;

    // Вычисляем общий размер блока: данные + заголовок, выровненный до 8 байт
    size_t totalSize = alignUp(userSize + HEADER_SIZE, ALIGNMENT);

    // Ищем первый подходящий свободный блок
    BlockHeader* block = findSuitableBlock(totalSize);
    if (!block) {
        cout << "Error: insufficient memory to allocate " << userSize << " bytes.\n";
        return nullptr;
    }

    // Определяем минимальный размер, при котором выгодно делить блок
    size_t minFreeSize = alignUp(HEADER_SIZE, ALIGNMENT);

    if (block->size > totalSize + minFreeSize) { // Блок достаточно велик для разделения
        // Удаляем текущий блок из freeList
        if (freeList == block) {
            freeList = block->nextFree;
        }
        else {
            BlockHeader* curr = freeList;
            while (curr && curr->nextFree != block)
                curr = curr->nextFree;
            if (curr) curr->nextFree = block->nextFree;
        }

        // Создаём новый свободный блок из остатка
        BlockHeader* newFree = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(block) + totalSize
        );
        newFree->size = block->size - totalSize;
        newFree->isAllocated = false;

        // Добавляем новый блок в начало `freeList` (для простоты)
        newFree->nextFree = freeList;
        freeList = newFree;

        // Обновляем размер исходного блока и помечаем его как занятый
        block->size = totalSize;
        block->nextFree = nullptr;
        block->isAllocated = true;
    }
    else { // Блок слишком мал для разделения — используем целиком
        // Удаляем блок из freeList обычным способом
        if (freeList == block) {
            freeList = block->nextFree;
        }
        else {
            BlockHeader* curr = freeList;
            while (curr && curr->nextFree != block)
                curr = curr->nextFree;
            if (curr) curr->nextFree = block->nextFree;
        }

        block->nextFree = nullptr;
        block->isAllocated = true;
    }

    // Возвращаем указатель на данные (сразу после заголовка)
    return ptrFromHeader(block);
}

void MemoryAllocator::deallocate(void* ptr) {
    if (!ptr || !isValidPtr(ptr)) { // Проверка корректности указателя
        cout << "Error: invalid pointer for deallocation.\n";
        return;
    }

    // Получаем заголовок блока по указателю на данные
    BlockHeader* block = headerFromPtr(ptr);

    if (!block->isAllocated) { // Проверка повторного освобождения
        cout << "Error: block is already free.\n";
        return;
    }

    // Помечаем блок как свободный и добавляем в начало freeList
    block->isAllocated = false;
    block->nextFree = freeList;
    freeList = block;

    // Перестраиваем список и сливаем соседние свободные блоки
    rebuildFreeList();
}

BlockHeader* MemoryAllocator::findSuitableBlock(size_t totalSize) {
    // Проход по списку свободных блоков в поиске первого подходящего (first-fit)
    BlockHeader* prev = nullptr;
    BlockHeader* curr = freeList;

    while (curr) {
        if (curr->size >= totalSize) {
            if (prev) { // Найден подходящий блок — удаляем его из `freeList`
                prev->nextFree = curr->nextFree;
            }
            else {
                freeList = curr->nextFree;
            }
            return curr;
        }
        prev = curr;
        curr = curr->nextFree;
    }
    return nullptr; // Подходящий блок не найден
}

void MemoryAllocator::rebuildFreeList() {
    // Полная реконструкция freeList с нуля
    freeList = nullptr;
    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);

    // Проходим по всей куче, блок за блоком
    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (!curr->isAllocated) {
            // Свободный блок добавляется в начало нового списка
            curr->nextFree = freeList;
            freeList = curr;
        }
        // Переход к следующему блоку по его размеру
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }
}

// === Вспомогательные утилиты ===

bool MemoryAllocator::isValidPtr(void* ptr) const {
    // Проверка, что ptr указывает на данные внутри кучи
    char* cptr = static_cast<char*>(ptr);
    return (cptr >= heapStart + HEADER_SIZE) && (cptr < heapStart + heapSize);
}

BlockHeader* MemoryAllocator::headerFromPtr(void* ptr) const {
    // Сдвиг назад на размер заголовка
    return reinterpret_cast<BlockHeader*>(
        static_cast<char*>(ptr) - HEADER_SIZE
    );
}

void* MemoryAllocator::ptrFromHeader(BlockHeader* header) const {
    // Сдвиг вперёд на размер заголовка — возвращаем указатель на данные
    return static_cast<void*>(
        reinterpret_cast<char*>(header) + HEADER_SIZE
    );
}

// === Методы для отладки и анализа ===

void MemoryAllocator::printStatus() const {
    // Сканирование всей кучи для сбора статистики
    size_t totalFree = 0, totalAllocated = 0;
    size_t freeBlocks = 0, allocatedBlocks = 0;
    size_t maxFree = 0;

    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);
    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (curr->isAllocated) {
            totalAllocated += curr->size;
            allocatedBlocks++;
        }
        else {
            totalFree += curr->size;
            freeBlocks++;
            if (curr->size > maxFree) maxFree = curr->size;
        }
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }

    // Расчёт внешней фрагментации по формуле: F = 1 - (`maxFree` / `totalFree`)
    double frag = (totalFree > 0) ? (1.0 - static_cast<double>(maxFree) / totalFree) : 0.0;

    // Вывод статистики
    cout << "\n=== Heap Status ===\n";
    cout << "Total size: " << heapSize << " bytes\n";
    cout << "Allocated: " << totalAllocated << " bytes (" 
              << (100.0 * totalAllocated / heapSize) << "%)\n";
    cout << "Free: " << totalFree << " bytes (" 
              << (100.0 * totalFree / heapSize) << "%)\n";
    cout << "Free blocks: " << freeBlocks << "\n";
    cout << "Allocated blocks: " << allocatedBlocks << "\n";
    cout << "External fragmentation: " << (frag * 100) << "%\n";
}

double MemoryAllocator::getFragmentation() const {
    // Аналогично `printStatus()`, но только расчёт фрагментации
    size_t totalFree = 0, maxFree = 0;
    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);
    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (!curr->isAllocated) {
            totalFree += curr->size;
            if (curr->size > maxFree) maxFree = curr->size;
        }
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }
    return (totalFree > 0) ? (1.0 - static_cast<double>(maxFree) / totalFree) : 0.0;
}