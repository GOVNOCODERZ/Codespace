#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <cstddef>
#include <cstdint>
#include <iostream>

struct BlockHeader {
    size_t size;               // Общий размер блока (включая заголовок), в байтах
    bool isAllocated;          // true — занят, false — свободен
    BlockHeader* nextFree;     // Только для свободных блоков
};

class MemoryAllocator {
public:
    explicit MemoryAllocator(size_t heapSize = 65536); // 64 КБ по умолчанию
    ~MemoryAllocator();

    void* allocate(size_t size);
    void deallocate(void* ptr);
    void printStatus() const;
    double getFragmentation() const;

private:

    static constexpr size_t ALIGNMENT = 8;
    static constexpr size_t HEADER_SIZE = sizeof(BlockHeader);

    char* heapStart;
    size_t heapSize;
    BlockHeader* freeList;

    size_t alignSize(size_t size) const;
    BlockHeader* findBlock(size_t requiredSize);
    void mergeFreeBlocks();
    BlockHeader* blockFromPtr(void* ptr) const;
    void* ptrFromBlock(BlockHeader* block) const;
    bool isValidPointer(void* ptr) const;
};

#endif // MEMORY_ALLOCATOR_H