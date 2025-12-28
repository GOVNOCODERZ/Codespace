#include "MemoryAllocator.h"
#include <iostream>
using namespace std;

// === Конструктор и деструктор ===

MemoryAllocator::MemoryAllocator(size_t heapSize)
    : heapSize(heapSize) {
    heapStart = new char[heapSize];
    // Инициализируем один большой свободный блок
    BlockHeader* first = reinterpret_cast<BlockHeader*>(heapStart);
    first->size = heapSize;
    first->isAllocated = false;
}

MemoryAllocator::~MemoryAllocator() {
    delete[] heapStart;
}

// === Вспомогательные методы ===

size_t MemoryAllocator::alignUp(size_t value, size_t alignment) const {
    return (value + alignment - 1) & ~(alignment - 1);
}

void MemoryAllocator::coalesceHeap() {
    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);
    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (!curr->isAllocated) {
            // Ищем следующие смежные свободные блоки и объединяем
            BlockHeader* next = reinterpret_cast<BlockHeader*>(
                reinterpret_cast<char*>(curr) + curr->size
            );
            while (next < reinterpret_cast<BlockHeader*>(heapStart + heapSize) &&
                   !next->isAllocated) {
                curr->size += next->size;
                next = reinterpret_cast<BlockHeader*>(
                    reinterpret_cast<char*>(curr) + curr->size
                );
            }
        }
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }
}

// === Основные методы аллокатора ===

void* MemoryAllocator::allocate(size_t userSize) {
    if (userSize == 0) return nullptr;

    size_t totalSize = alignUp(userSize + HEADER_SIZE, ALIGNMENT);
    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);

    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (!curr->isAllocated && curr->size >= totalSize) {
            size_t minSplitSize = alignUp(HEADER_SIZE, ALIGNMENT);
            if (curr->size >= totalSize + minSplitSize) {
                BlockHeader* newBlock = reinterpret_cast<BlockHeader*>(
                    reinterpret_cast<char*>(curr) + totalSize
                );
                newBlock->size = curr->size - totalSize;
                newBlock->isAllocated = false;

                curr->size = totalSize;
            }
            curr->isAllocated = true;
            return ptrFromHeader(curr);
        }
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }

    cout << "Error: insufficient memory to allocate " << userSize << " bytes.\n";
    return nullptr;
}

void MemoryAllocator::deallocate(void* ptr) {
    if (!ptr || !isValidPtr(ptr)) {
        cout << "Error: invalid pointer for deallocation.\n";
        return;
    }

    BlockHeader* block = headerFromPtr(ptr);
    if (!block->isAllocated) {
        cout << "Error: block is already free.\n";
        return;
    }

    block->isAllocated = false;
}

// === Вспомогательные утилиты ===

bool MemoryAllocator::isValidPtr(void* ptr) const {
    char* cptr = static_cast<char*>(ptr);
    return (cptr >= heapStart + HEADER_SIZE) && (cptr < heapStart + heapSize);
}

BlockHeader* MemoryAllocator::headerFromPtr(void* ptr) const {
    return reinterpret_cast<BlockHeader*>(
        static_cast<char*>(ptr) - HEADER_SIZE
    );
}

void* MemoryAllocator::ptrFromHeader(BlockHeader* header) const {
    return static_cast<void*>(
        reinterpret_cast<char*>(header) + HEADER_SIZE
    );
}

// === Методы для отладки и анализа ===

void MemoryAllocator::printStatus() const {
    // Для корректного подсчёта фрагментации сначала сольём блоки
    MemoryAllocator* nonConstThis = const_cast<MemoryAllocator*>(this);
    nonConstThis->coalesceHeap();

    size_t totalFree = 0, totalAllocated = 0;
    size_t freeBlocks = 0, allocatedBlocks = 0;
    size_t maxFree = 0;

    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);
    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (curr->isAllocated) {
            totalAllocated += curr->size;
            allocatedBlocks++;
        } else {
            totalFree += curr->size;
            freeBlocks++;
            if (curr->size > maxFree) maxFree = curr->size;
        }
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }

    double frag = (totalFree > 0) ? (1.0 - static_cast<double>(maxFree) / totalFree) : 0.0;

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
    MemoryAllocator* nonConstThis = const_cast<MemoryAllocator*>(this);
    nonConstThis->coalesceHeap();

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