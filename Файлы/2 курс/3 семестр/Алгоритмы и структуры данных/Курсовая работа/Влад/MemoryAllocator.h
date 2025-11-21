#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <cstddef>
#include <iostream>

// Заголовок блока — теперь глобальный тип
struct BlockHeader {
    size_t size;          // Размер всего блока (включая заголовок)
    bool isAllocated;     // true — занят, false — свободен
    BlockHeader* nextFree; // Только для свободных блоков
};

class MemoryAllocator {
public:
    explicit MemoryAllocator(size_t heapSize = 65536);
    ~MemoryAllocator();

    void* allocate(size_t userSize);
    void deallocate(void* ptr);
    void printStatus() const;
    double getFragmentation() const;

private:
    static constexpr size_t ALIGNMENT = 8;
    static constexpr size_t HEADER_SIZE = sizeof(BlockHeader);

    char* heapStart;
    size_t heapSize;
    BlockHeader* freeList;

    size_t alignUp(size_t value, size_t alignment) const;
    BlockHeader* findSuitableBlock(size_t totalSize);
    void rebuildFreeList();
    bool isValidPtr(void* ptr) const;
    BlockHeader* headerFromPtr(void* ptr) const;
    void* ptrFromHeader(BlockHeader* header) const;
};

#endif // MEMORY_ALLOCATOR_H