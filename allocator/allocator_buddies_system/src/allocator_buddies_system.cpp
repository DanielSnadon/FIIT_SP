#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"

// Тесты запускаются из корня:
// ./build/allocator/allocator_buddies_system/tests/sys_prog_allctr_allctr_bdds_sstm_tests

#pragma region HelperFuncs

// Align.

constexpr size_t alignUp(size_t value, size_t alignment) noexcept
{
    return (value + alignment - 1) / alignment * alignment;
}

constexpr size_t defaultAlign = alignof(std::max_align_t);

#pragma region Offsets

// Аллокатор MetaData
// [Родит.Аллокатор*][фитмод][размер выдел. памяти][мьютекс][первый своб. блок*]

constexpr size_t parentAllocatorOffset = 0;

constexpr size_t fitModeOffset = alignUp(parentAllocatorOffset + sizeof(std::pmr::memory_resource*), alignof(allocator_with_fit_mode::fit_mode));

constexpr size_t totalSizeOffset = alignUp(fitModeOffset + sizeof(allocator_with_fit_mode::fit_mode), alignof(size_t));

constexpr size_t mutexOffset = alignUp(totalSizeOffset + sizeof(size_t), alignof(std::mutex));

constexpr size_t firstFreeOffset = alignUp(mutexOffset + sizeof(std::mutex), alignof(void*));

constexpr size_t allocatorMetaSize = alignUp(firstFreeOffset + sizeof(void*), defaultAlign);

// Занятого MetaData 
// [Метаданные Аллокатора*][size]

// Свободного MetaData
// [Следующий свободный*][size]

constexpr size_t blockOffset = 0;

constexpr size_t blockSizeOffset = alignUp(blockOffset + sizeof(void*), alignof(size_t));

constexpr size_t blockMetaSize = alignUp(blockSizeOffset + sizeof(size_t), defaultAlign);

#pragma endregion

// Бинарное - related

// Нахождение ближайшего числа 2^p
size_t binarize(size_t value) noexcept
{
    size_t result = 1;

    while (result < value)
    {
        result <<= 1;
    }

    return result;
}

const size_t minBlockSize = binarize(blockMetaSize);

size_t roundBuddySize(size_t value) noexcept
{
    return binarize(value);
}

size_t requiredBlockSize(size_t user_size) noexcept
{
    size_t result = roundBuddySize(blockMetaSize + user_size);

    if (result < minBlockSize)
    {
        result = minBlockSize;
    }

    return result;
}

bool isPower2(size_t value) noexcept
{
    return value != 0 && (value & (value - 1)) == 0;
}

#pragma region accesses

// Конвертация указателя в байтовый

std::byte* ptrToBytes(void* ptr) noexcept
{
    return reinterpret_cast<std::byte*>(ptr);
}

const std::byte* ptrToBytes(const void*ptr) noexcept
{
    return reinterpret_cast<const std::byte*>(ptr);
}

// Получить поле из offset

template<typename T>
T &accessField(void* base, size_t offset) noexcept
{
    return *reinterpret_cast<T*>(ptrToBytes(base) + offset);
}

template<typename T>
const T &accessField(const void* base, size_t offset) noexcept
{
    return *reinterpret_cast<const T*>(ptrToBytes(base) + offset);
}

// Доступ к аллокатору

std::pmr::memory_resource *&accessParent(void *metaStart) noexcept
{
    return accessField<std::pmr::memory_resource*>(metaStart, parentAllocatorOffset);
}

const std::pmr::memory_resource *accessParent(const void *metaStart) noexcept
{
    return accessField<const std::pmr::memory_resource*>(metaStart, parentAllocatorOffset);
}

allocator_with_fit_mode::fit_mode &accessFitMode(void *metaStart) noexcept
{
    return accessField<allocator_with_fit_mode::fit_mode>(metaStart, fitModeOffset);
}

allocator_with_fit_mode::fit_mode accessFitMode(const void *metaStart) noexcept
{
    return accessField<allocator_with_fit_mode::fit_mode>(metaStart, fitModeOffset);
}

size_t &accessTotalSize(void *metaStart) noexcept
{
    return accessField<size_t>(metaStart, totalSizeOffset);
}

size_t accessTotalSize(const void *metaStart) noexcept
{
    return accessField<size_t>(metaStart, totalSizeOffset);
}

std::mutex &accessMutex(void *metaStart) noexcept
{
    return accessField<std::mutex>(metaStart, mutexOffset);
}

const std::mutex &accessMutex(const void *metaStart) noexcept
{
    return accessField<const std::mutex>(metaStart, mutexOffset);
}

void* &accessFirstFreeBlock(void *metaStart) noexcept
{
    return accessField<void*>(metaStart, firstFreeOffset);
}

void* accessFirstFreeBlock(const void *metaStart) noexcept
{
    return accessField<void*>(metaStart, firstFreeOffset);
}

size_t accessSizeX(const void *metaStart) noexcept
{
    return allocatorMetaSize + accessTotalSize(metaStart);
}

void* accessFirstBlock(void *metaStart) noexcept
{
    return ptrToBytes(metaStart) + allocatorMetaSize;
}

const void* accessFirstBlock(const void *metaStart) noexcept
{
    return ptrToBytes(metaStart) + allocatorMetaSize;
}

void *accessEnd(void *metaStart) noexcept
{
    return ptrToBytes(accessFirstBlock(metaStart)) + accessTotalSize(metaStart);
}

const void *accessEnd(const void *metaStart) noexcept
{
    return ptrToBytes(accessFirstBlock(metaStart)) + accessTotalSize(metaStart);
}

// Доступ к блоку

void* &accessBlockPtrField(void *block) noexcept
{
    return accessField<void*>(block, blockOffset);
}

void* accessBlockPtrField(const void *block) noexcept
{
    return accessField<void*>(block, blockOffset);
}

size_t &accessBlockSize(void *block) noexcept
{
    return accessField<size_t>(block, blockSizeOffset);
}

size_t accessBlockSize(const void *block) noexcept
{
    return accessField<size_t>(block, blockSizeOffset);
}

void* &accessNextFree(void *block) noexcept
{
    return accessBlockPtrField(block);
}

void* accessNextFree(const void *block) noexcept
{
    return accessBlockPtrField(block);
}

void* &accessBlockOwner(void *block) noexcept
{
    return accessBlockPtrField(block);
}

void* accessBlockOwner(const void *block) noexcept
{
    return accessBlockPtrField(block);
}

// Other

bool isOccupied(const void *block, const void *metaStart) noexcept
{
    return accessBlockOwner(block) == metaStart;
}

void markOccupied(void *block, size_t blockSize, void *metaStart) noexcept
{
    accessBlockOwner(block) = metaStart;
    accessBlockSize(block) = blockSize;
}

void markFree(void *block, size_t blockSize, void *nextFree) noexcept
{
    accessNextFree(block) = nextFree;
    accessBlockSize(block) = blockSize;
}

void* nextBlock(void *block) noexcept
{
    return ptrToBytes(block) + accessBlockSize(block);
}

const void* nextBlock(const void *block) noexcept
{
    return ptrToBytes(block) + accessBlockSize(block);
}

void* accessBlockSpace(void *block) noexcept
{
    return ptrToBytes(block) + blockMetaSize;
}

const void* accessBlockSpace(const void *block) noexcept
{
    return ptrToBytes(block) + blockMetaSize;
}

void* buddyOf(void *block, void *metaStart) noexcept
{
    const size_t currBlockSize = accessBlockSize(block);
    const size_t beginOffset = ptrToBytes(block) - ptrToBytes(accessFirstBlock(metaStart));
    const size_t buddyOffset = beginOffset ^ currBlockSize;

    return ptrToBytes(accessFirstBlock(metaStart)) + buddyOffset;
}

#pragma endregion

#pragma region FreeList

void insertFreeBlock(void *block, void *metaStart) noexcept
{
    accessNextFree(block) = nullptr;

    void* &head = accessFirstFreeBlock(metaStart);

    if (head == nullptr || head > block)
    {
        accessNextFree(block) = head;
        head = block;
        return;
    }

    void* current = head;

    while (accessNextFree(current) != nullptr && accessNextFree(current) < block)
    {
        current = accessNextFree(current);
    }

    accessNextFree(block) = accessNextFree(current);
    accessNextFree(current) = block;
}

void removeFreeBlock(void *block, void *metaStart) noexcept
{
    void* &head = accessFirstFreeBlock(metaStart);

    if (head == nullptr)
    {
        return;
    }

    if (head == block)
    {
        head = accessNextFree(head);
        accessNextFree(block) = nullptr;
        return;
    }

    void* current = head;

    while (accessNextFree(current) != nullptr && accessNextFree(current) != block)
    {
        current = accessNextFree(current);
    }

    if (accessNextFree(current) == block)
    {
        accessNextFree(current) = accessNextFree(block);
        accessNextFree(block) = nullptr;
    }
}

bool isBlockInList(const void *block, const void *metaStart) noexcept
{
    for (void*current = accessFirstFreeBlock(metaStart); current != nullptr; current = accessNextFree(current))
    {
        if (current == block)
        {
            return true;
        }
    }

    return false;
}

void* chooseFreeBlock(void *metaStart, size_t neededSize) noexcept
{
    const auto fitMode = accessFitMode(metaStart);

    void* theBlock = nullptr;

    for (void* current = accessFirstFreeBlock(metaStart); current != nullptr; current = accessNextFree(current))
    {
        const size_t currSize = accessBlockSize(current);

        if (currSize < neededSize)
        {
            continue;
        }

        if (theBlock == nullptr)
        {
            theBlock = current;

            if (fitMode == allocator_with_fit_mode::fit_mode::first_fit)
            {
                break;
            }

            continue;
        }

        if (fitMode == allocator_with_fit_mode::fit_mode::the_best_fit &&
            currSize < accessBlockSize(theBlock))
        {
            theBlock = current;
        }
        else if (fitMode == allocator_with_fit_mode::fit_mode::the_worst_fit &&
                    currSize > accessBlockSize(theBlock))
        {
            theBlock = current;
        }
    }

    return theBlock;
}

#pragma endregion

#pragma region Memory

void* allocateMemoryBlock(size_t size, std::pmr::memory_resource *parentAllocator)
{
    if (parentAllocator != nullptr)
    {
        return parentAllocator->allocate(size, defaultAlign);
    }

    return ::operator new(size);
}

void releaseMemoryBlock(void *metaStart) noexcept
{
    if (metaStart == nullptr)
    {
        return;
    }

    std::pmr::memory_resource* parentAllocator = accessParent(metaStart);
    const size_t totalSize = accessSizeX(metaStart);

    accessMutex(metaStart).~mutex();

    if (parentAllocator != nullptr)
    {
        parentAllocator->deallocate(metaStart, totalSize, defaultAlign);
    }
    else
    {
        ::operator delete(metaStart);
    }
}

void initializeMemoryBlock(void *metaStart, std::pmr::memory_resource *parentAllocator, allocator_with_fit_mode::fit_mode fitMode, size_t spaceSize)
{
    accessParent(metaStart) = parentAllocator;
    accessFitMode(metaStart) = fitMode;
    accessTotalSize(metaStart) = spaceSize;
    new (&accessMutex(metaStart)) std::mutex();

    void *freeBlockZero = accessFirstBlock(metaStart);
    markFree(freeBlockZero, spaceSize, nullptr);
    accessFirstFreeBlock(metaStart) = freeBlockZero;
}

void *translatePtr(void *oldPtr, const void *oldMetaStart, void *newMetaStart) noexcept
{
    if (oldPtr == nullptr)
    {
        return nullptr;
    }

    return ptrToBytes(newMetaStart) +
            (ptrToBytes(oldPtr) - ptrToBytes(oldMetaStart));
}

void *findOccupiedBlockByPtr(void *at, void *metaStart) noexcept
{
    for (void *current = accessFirstBlock(metaStart); current < accessEnd(metaStart); current = nextBlock(current))
    {
        if (isOccupied(current, metaStart) && accessBlockSpace(current) == at)
        {
            return current;
        }
    }

    return nullptr;
}

#pragma endregion

#pragma endregion

#pragma region TaskArea

allocator_buddies_system::~allocator_buddies_system()
{
    releaseMemoryBlock(_trusted_memory);
    _trusted_memory = nullptr;
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system &&other) noexcept : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
}

allocator_buddies_system &allocator_buddies_system::operator=(
    allocator_buddies_system &&other) noexcept
{
    if (this != &other)
    {
        releaseMemoryBlock(_trusted_memory);
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }

    return *this;
}

allocator_buddies_system::allocator_buddies_system(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode) : _trusted_memory(nullptr)
{
    if (parent_allocator == nullptr)
    {
        parent_allocator = std::pmr::get_default_resource();
    }

    if (space_size == 0)
    {
        throw std::logic_error("Не удалось создать аллокатор с нулевым размером.");
    }

    const size_t roundedSpaceSize = roundBuddySize(space_size);

    if (roundedSpaceSize < minBlockSize)
    {
        throw std::logic_error("Не удалось создать аллокатор с размером, меньшим допустимого значения.");
    }

    const size_t totalSize = allocatorMetaSize + roundedSpaceSize;
    _trusted_memory = allocateMemoryBlock(totalSize, parent_allocator);

    try
    {
        initializeMemoryBlock(_trusted_memory, parent_allocator, allocate_fit_mode, roundedSpaceSize);
    }
    catch (...)
    {
        if (parent_allocator != nullptr)
        {
            parent_allocator->deallocate(_trusted_memory, totalSize, defaultAlign);
        }
        else
        {
            ::operator delete(_trusted_memory);
        }

        _trusted_memory = nullptr;
        throw;
    }
    
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(
    size_t size)
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));

    const size_t neededSize = requiredBlockSize(size);

    if (neededSize > accessTotalSize(_trusted_memory))
    {
        throw std::bad_alloc();
    }

    void *chosenBlock = chooseFreeBlock(_trusted_memory, neededSize);

    if (chosenBlock == nullptr)
    {
        throw std::bad_alloc();
    }

    removeFreeBlock(chosenBlock, _trusted_memory);

    while (accessBlockSize(chosenBlock) > neededSize)
    {
        const size_t newSize = accessBlockSize(chosenBlock) / 2;

        void *rightBuddy = ptrToBytes(chosenBlock) + newSize;

        markFree(chosenBlock, newSize, nullptr);

        markFree(rightBuddy, newSize, nullptr);
        insertFreeBlock(rightBuddy, _trusted_memory);
    }

    markOccupied(chosenBlock, neededSize, _trusted_memory);

    return accessBlockSpace(chosenBlock);
}

void allocator_buddies_system::do_deallocate_sm(void *at)
{
    if (at == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));

    void *block = findOccupiedBlockByPtr(at, _trusted_memory);

    if (block == nullptr)
    {
        throw std::invalid_argument("Блок не принадлежит данному аллокатору.");
    }

    size_t currBlockSize = accessBlockSize(block);

    markFree(block, currBlockSize, nullptr);

    while (currBlockSize < accessTotalSize(_trusted_memory))
    {
        void *buddy = buddyOf(block, _trusted_memory);

        if (!isBlockInList(buddy, _trusted_memory) || accessBlockSize(buddy) != currBlockSize)
        {
            break;
        }

        removeFreeBlock(buddy, _trusted_memory);

        if (buddy < block)
        {
            block = buddy;
        }

        currBlockSize *= 2;
        markFree(block, currBlockSize, nullptr);
    }

    insertFreeBlock(block, _trusted_memory);
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)
{
    if (other._trusted_memory == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(accessMutex(other._trusted_memory));

    const size_t totalSize = accessSizeX(other._trusted_memory);
    std::pmr::memory_resource *parentAllocator = accessParent(other._trusted_memory);

    _trusted_memory = allocateMemoryBlock(totalSize, parentAllocator);

    try
    {
        std::memcpy(_trusted_memory, other._trusted_memory, totalSize);

        accessMutex(_trusted_memory).~mutex();
        new (&accessMutex(_trusted_memory)) std::mutex();

        accessFirstFreeBlock(_trusted_memory) = translatePtr(accessFirstBlock(_trusted_memory), other._trusted_memory, _trusted_memory);
        
        for (void *current = accessFirstBlock(_trusted_memory); current < accessEnd(_trusted_memory); current = nextBlock(current))
        {
            const size_t currentSize = accessBlockSize(current);

            if (!isPower2(currentSize))
            {
                throw std::logic_error("Ошибка размера после копирования.");
            }

            if (isOccupied(current, other._trusted_memory))
            {
                accessBlockOwner(current) = _trusted_memory;
            }
            else
            {
                accessNextFree(current) = translatePtr(accessNextFree(current), other._trusted_memory, _trusted_memory);
            }
        }
    }
    catch (...)
    {
        releaseMemoryBlock(_trusted_memory);
        _trusted_memory = nullptr;
        throw;
    }
}

allocator_buddies_system &allocator_buddies_system::operator=(const allocator_buddies_system &other)
{
    if (this == &other)
    {
        return *this;
    }

    allocator_buddies_system temp = other;
    *this = std::move(temp);

    return *this;
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
    accessFitMode(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
        return get_blocks_info_inner();
    }
    catch (...)
    {
        return {};
    }
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> result;

    for (void *curr = accessFirstBlock(_trusted_memory); curr < accessEnd(_trusted_memory); curr = nextBlock(curr))
    {
        allocator_test_utils::block_info info;
        info.block_size = accessBlockSize(curr);
        info.is_block_occupied = isOccupied(curr, _trusted_memory);
        result.push_back(info);
    }

    return result;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return {};
    }

    return buddy_iterator(accessFirstBlock(_trusted_memory));
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return {};
    }

    return buddy_iterator(const_cast<void*> (accessEnd(_trusted_memory)));
}

bool allocator_buddies_system::buddy_iterator::operator==(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_buddies_system::buddy_iterator &allocator_buddies_system::buddy_iterator::operator++() & noexcept
{
    _block = nextBlock(_block);
    return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int n)
{
    buddy_iterator copy = *this;
    ++(*this);
    
    return copy;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept
{
    return accessBlockSize(_block);
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    const void *firstValue = accessBlockPtrField(_block);

    return firstValue != nullptr && firstValue < _block;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    return _block;
}

allocator_buddies_system::buddy_iterator::buddy_iterator(void *start) : _block(start)
{
}

allocator_buddies_system::buddy_iterator::buddy_iterator() : _block(nullptr)
{
}

#pragma endregion