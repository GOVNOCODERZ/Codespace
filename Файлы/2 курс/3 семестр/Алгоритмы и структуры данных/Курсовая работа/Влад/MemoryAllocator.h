#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <cstddef>
#include <iostream>

struct BlockHeader {
    size_t size;
    bool isAllocated;
    // Поле `nextFree` больше не нужно!
};

class MemoryAllocator {
public:
    explicit MemoryAllocator(size_t heapSize = 65536);
    ~MemoryAllocator();

    void* allocate(size_t userSize);
    void deallocate(void* ptr);
    void printStatus() const;
    double getFragmentation() const;

    BlockHeader* headerFromPtr(void* ptr) const;
    bool isValidPtr(void* ptr) const;
    void* ptrFromHeader(BlockHeader* header) const;

private:
    static constexpr size_t ALIGNMENT = 8;
    static constexpr size_t HEADER_SIZE = sizeof(BlockHeader);

    char* heapStart;
    size_t heapSize;

    size_t alignUp(size_t value, size_t alignment) const;
    void coalesceHeap(); // Новый метод для слияния при анализе

};

#endif // MEMORY_ALLOCATOR_H