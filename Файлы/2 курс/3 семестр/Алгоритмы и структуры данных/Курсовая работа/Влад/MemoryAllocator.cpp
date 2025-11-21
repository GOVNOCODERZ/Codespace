#include "MemoryAllocator.h"
#include <cstring>
#include <algorithm>
#include <iostream>

MemoryAllocator::MemoryAllocator(size_t heapSize)
    : heapSize(heapSize), freeList(nullptr) {
    heapStart = new char[heapSize];
    BlockHeader* first = reinterpret_cast<BlockHeader*>(heapStart);
    first->size = heapSize;
    first->isAllocated = false;
    first->nextFree = nullptr;
    freeList = first;
}

MemoryAllocator::~MemoryAllocator() {
    delete[] heapStart;
}

size_t MemoryAllocator::alignUp(size_t value, size_t alignment) const {
    return (value + alignment - 1) & ~(alignment - 1);
}

void* MemoryAllocator::allocate(size_t userSize) {
    if (userSize == 0) return nullptr;

    size_t totalSize = alignUp(userSize + HEADER_SIZE, ALIGNMENT);
    BlockHeader* block = findSuitableBlock(totalSize);

    if (!block) {
        std::cout << "Error: insufficient memory to allocate " << userSize << " bytes.\n";
        return nullptr;
    }

    // Если остаток достаточно велик — делим
    size_t minFreeSize = alignUp(HEADER_SIZE, ALIGNMENT);
    if (block->size > totalSize + minFreeSize) {
        // Удалить текущий блок из freeList
        if (freeList == block) {
            freeList = block->nextFree;
        } else {
            BlockHeader* curr = freeList;
            while (curr && curr->nextFree != block)
                curr = curr->nextFree;
            if (curr) curr->nextFree = block->nextFree;
        }

        // Создать новый свободный блок
        BlockHeader* newFree = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(block) + totalSize
        );
        newFree->size = block->size - totalSize;
        newFree->isAllocated = false;

        // Добавить новый блок в начало freeList
        newFree->nextFree = freeList;
        freeList = newFree;

        // Установить размер текущего блока
        block->size = totalSize;
        block->nextFree = nullptr;
        block->isAllocated = true;
    } else {
        // Удалить из freeList
        if (freeList == block) {
            freeList = block->nextFree;
        } else {
            BlockHeader* curr = freeList;
            while (curr && curr->nextFree != block)
                curr = curr->nextFree;
            if (curr) curr->nextFree = block->nextFree;
        }

        block->size = totalSize;
        block->nextFree = nullptr;
        block->isAllocated = true;
    }

    return ptrFromHeader(block);
}

void MemoryAllocator::deallocate(void* ptr) {
    if (!ptr || !isValidPtr(ptr)) {
        std::cout << "Error: invalid pointer for deallocation.\n";
        return;
    }

    BlockHeader* block = headerFromPtr(ptr);
    if (!block->isAllocated) {
        std::cout << "Error: block is already free.\n";
        return;
    }

    block->isAllocated = false;
    block->nextFree = freeList;
    freeList = block;

    // Перестроить список и слить соседей
    rebuildFreeList();
}

BlockHeader* MemoryAllocator::findSuitableBlock(size_t totalSize) {
    BlockHeader* prev = nullptr;
    BlockHeader* curr = freeList;

    while (curr) {
        if (curr->size >= totalSize) {
            if (prev) {
                prev->nextFree = curr->nextFree;
            } else {
                freeList = curr->nextFree;
            }
            return curr;
        }
        prev = curr;
        curr = curr->nextFree;
    }
    return nullptr;
}

void MemoryAllocator::rebuildFreeList() {
    // Проходим по куче и собираем все свободные блоки
    freeList = nullptr;
    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);
    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (!curr->isAllocated) {
            curr->nextFree = freeList;
            freeList = curr;
        }
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }
}

bool MemoryAllocator::isValidPtr(void* ptr) const {
    if (ptr < reinterpret_cast<void*>(heapStart + HEADER_SIZE)) return false;
    if (ptr >= reinterpret_cast<void*>(heapStart + heapSize)) return false;
    return true;
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

void MemoryAllocator::printStatus() const {
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

    std::cout << "\n=== Heap Status ===\n";
    std::cout << "Total size: " << heapSize << " bytes\n";
    std::cout << "Allocated: " << totalAllocated << " bytes (" 
              << (100.0 * totalAllocated / heapSize) << "%)\n";
    std::cout << "Free: " << totalFree << " bytes (" 
              << (100.0 * totalFree / heapSize) << "%)\n";
    std::cout << "Free blocks: " << freeBlocks << "\n";
    std::cout << "Allocated blocks: " << allocatedBlocks << "\n";
    std::cout << "External fragmentation: " << (frag * 100) << "%\n";
}

double MemoryAllocator::getFragmentation() const {
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