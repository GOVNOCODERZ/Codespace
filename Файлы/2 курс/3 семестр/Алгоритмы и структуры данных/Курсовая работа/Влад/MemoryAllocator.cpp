#include "MemoryAllocator.h"
#include <cstring>
#include <algorithm>
#include <cmath>

MemoryAllocator::MemoryAllocator(size_t heapSize)
    : heapSize(heapSize), freeList(nullptr) {
    heapStart = new char[heapSize];
    BlockHeader* firstBlock = reinterpret_cast<BlockHeader*>(heapStart);
    firstBlock->size = heapSize;
    firstBlock->isAllocated = false;
    firstBlock->nextFree = nullptr;
    freeList = firstBlock;
}

MemoryAllocator::~MemoryAllocator() {
    delete[] heapStart;
}

size_t MemoryAllocator::alignSize(size_t size) const {
    size_t remainder = size % ALIGNMENT;
    if (remainder == 0) return size;
    return size + (ALIGNMENT - remainder);
}

void* MemoryAllocator::allocate(size_t userSize) {
    if (userSize == 0) return nullptr;

    size_t totalSize = alignSize(userSize + HEADER_SIZE);
    BlockHeader* block = findBlock(totalSize);

    if (!block) {
        std::cout << "Error: insufficient memory to allocate " << userSize << " bytes.\n";
        return nullptr;
    }

    // Разделение блока, если он значительно больше
    if (block->size >= totalSize + alignSize(HEADER_SIZE + 1)) {
        size_t remaining = block->size - totalSize;
        BlockHeader* newBlock = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(block) + totalSize
        );
        newBlock->size = remaining;
        newBlock->isAllocated = false;

        // Вставить новый свободный блок в список
        newBlock->nextFree = block->nextFree;
        if (freeList == block) {
            freeList = newBlock;
        } else {
            BlockHeader* curr = freeList;
            while (curr && curr->nextFree != block) {
                curr = curr->nextFree;
            }
            if (curr) curr->nextFree = newBlock;
        }

        block->size = totalSize;
        block->nextFree = nullptr;
    } else {
        // Удалить блок из списка свободных
        if (freeList == block) {
            freeList = block->nextFree;
        } else {
            BlockHeader* curr = freeList;
            while (curr && curr->nextFree != block) {
                curr = curr->nextFree;
            }
            if (curr) curr->nextFree = block->nextFree;
        }
        block->nextFree = nullptr;
    }

    block->isAllocated = true;
    return ptrFromBlock(block);
}

void MemoryAllocator::deallocate(void* ptr) {
    if (!ptr) return;
    if (!isValidPointer(ptr)) {
        std::cout << "Error: attempt to deallocate invalid address.\n";
        return;
    }

    BlockHeader* block = blockFromPtr(ptr);
    if (block->isAllocated == false) {
        std::cout << "Error: block is already free.\n";
        return;
    }

    block->isAllocated = false;
    block->nextFree = freeList;
    freeList = block;

    mergeFreeBlocks();
}

BlockHeader* MemoryAllocator::findBlock(size_t requiredSize) {
    BlockHeader* prev = nullptr;
    BlockHeader* curr = freeList;

    while (curr) {
        if (curr->size >= requiredSize) {
            // Удалить из списка
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

void MemoryAllocator::mergeFreeBlocks() {
    BlockHeader* curr = reinterpret_cast<BlockHeader*>(heapStart);
    while (curr < reinterpret_cast<BlockHeader*>(heapStart + heapSize)) {
        if (!curr->isAllocated) {
            BlockHeader* next = reinterpret_cast<BlockHeader*>(
                reinterpret_cast<char*>(curr) + curr->size
            );
            while (next < reinterpret_cast<BlockHeader*>(heapStart + heapSize) &&
                   !next->isAllocated) {
                curr->size += next->size;
                next = reinterpret_cast<BlockHeader*>(
                    reinterpret_cast<char*>(next) + next->size
                );
            }
        }
        curr = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(curr) + curr->size
        );
    }

    // Перестроить freeList
    freeList = nullptr;
    curr = reinterpret_cast<BlockHeader*>(heapStart);
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

BlockHeader* MemoryAllocator::blockFromPtr(void* ptr) const {
    return reinterpret_cast<BlockHeader*>(static_cast<char*>(ptr) - HEADER_SIZE);
}

void* MemoryAllocator::ptrFromBlock(BlockHeader* block) const {
    return static_cast<void*>(reinterpret_cast<char*>(block) + HEADER_SIZE);
}

bool MemoryAllocator::isValidPointer(void* ptr) const {
    if (ptr < reinterpret_cast<void*>(heapStart + HEADER_SIZE)) return false;
    if (ptr >= reinterpret_cast<void*>(heapStart + heapSize)) return false;
    return true;
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
    std::cout << "External fragmentation: " 
              << (frag * 100) << "%\n";
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