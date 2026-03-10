#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

// Тесты запускаются из корня:
// ./build/allocator/allocator_boundary_tags/tests/sys_prog_allctr_allctr_bndr_tgs_tests 

// Align.

constexpr size_t alignUp(size_t value, size_t alignment) noexcept
{
    return (value + alignment - 1) / alignment * alignment;
}

constexpr size_t defaultAlign = alignof(std::max_align_t);

// Оффсеты - метаданные аллокатора
// [Родительский аллокатор][ФитМод][Размер][Мьютекс][ПервыйЗанятый]

constexpr size_t parentAllocatorOffset = 0;

constexpr size_t fitModeOffset = alignUp(parentAllocatorOffset + sizeof(std::pmr::memory_resource *), alignof(allocator_with_fit_mode::fit_mode));

constexpr size_t totalSizeOffset = alignUp(fitModeOffset + sizeof(allocator_with_fit_mode::fit_mode), alignof(size_t));

constexpr size_t mutexOffset = alignUp(totalSizeOffset + sizeof(size_t), alignof(std::mutex));

constexpr size_t firstOccupiedBlockOffset = alignUp(mutexOffset + sizeof(std::mutex), alignof(void *));
// !Заголовочное
constexpr size_t realAllocatorMetaSize = firstOccupiedBlockOffset + sizeof(void*);

// Оффсеты - занятый блок
// [Размер][Владелец][ПредЗанятый][СледЗанятый]

constexpr size_t blockSizeOffset = 0;

constexpr size_t blockOwnerOffset = alignUp(blockSizeOffset + sizeof(size_t), alignof(void *));

constexpr size_t blockPrevOffset = alignUp(blockOwnerOffset + sizeof(void *), alignof(void *));

constexpr size_t blockNextOffset = alignUp(blockPrevOffset + sizeof(void *), alignof(void *));
// !Заголовочное
constexpr size_t realOccupiedBlockMetaSize = sizeof(size_t) + sizeof(void*) + sizeof(void*) + sizeof(void*);


// Минимальный размер полезной нагрузки

constexpr size_t minimalOccupiedSize = defaultAlign;


// Конвертация указателя в байтовый

std::byte *ptrToBytes(void *ptr) noexcept
{
    return reinterpret_cast<std::byte*>(ptr);
}

const std::byte *ptrToBytes(const void *ptr) noexcept
{
    return reinterpret_cast<const std::byte*>(ptr);
}


// Получить поле из offset

template<typename T>
T &accessField(void *base, size_t offset)
{
    return *reinterpret_cast<T*>(ptrToBytes(base) + offset);
}

template<typename T>
const T &accessField(const void *base, size_t offset)
{
    return *reinterpret_cast<const T*>(ptrToBytes(base) + offset);
}

// Доступы

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

void *&accessFirstOccupiedBlock(void *metaStart) noexcept
{
    return accessField<void*>(metaStart, firstOccupiedBlockOffset);
}

const void *accessFirstOccupiedBlock(const void *metaStart) noexcept
{
    return accessField<const void*>(metaStart, firstOccupiedBlockOffset);
}

void *accessFirstBlock(void *metaStart) noexcept
{
    return ptrToBytes(metaStart) + realAllocatorMetaSize;
}

const void *accessFirstBlock(const void *metaStart) noexcept
{
    return ptrToBytes(metaStart) + realAllocatorMetaSize;
}

void *accessEnd(void *metaStart) noexcept
{
    return ptrToBytes(metaStart) + accessTotalSize(metaStart);
}

const void *accessEnd(const void *metaStart) noexcept
{
    return ptrToBytes(metaStart) + accessTotalSize(metaStart);
}

// Доступ к блоку

size_t &accessBlockSize(void *block) noexcept
{
    return accessField<size_t>(block, blockSizeOffset);
}

size_t accessBlockSize(const void *block) noexcept
{
    return accessField<size_t>(block, blockSizeOffset);
}

void *&accessBlockOwner(void *block) noexcept
{
    return accessField<void*>(block, blockOwnerOffset);
}

const void *accessBlockOwner(const void *block) noexcept
{
    return accessField<const void*>(block, blockOwnerOffset);
}

void *&accessPrevOccupied(void *block) noexcept
{
    return accessField<void*>(block, blockPrevOffset);
}

const void *accessPrevOccupied(const void *block) noexcept
{
    return accessField<const void*>(block, blockPrevOffset);
}

void *&accessNextOccupied(void *block) noexcept
{
    return accessField<void*>(block, blockNextOffset);
}

const void *accessNextOccupied(const void *block) noexcept
{
    return accessField<const void*>(block, blockNextOffset);
}

void *accessBlockSpace(void *block) noexcept
{
    return ptrToBytes(block) + realOccupiedBlockMetaSize;
}

const void *accessBlockSpace(const void *block) noexcept
{
    return ptrToBytes(block) + realOccupiedBlockMetaSize;
}

void *blockAdressFromFreeSpace(void *blockSpace) noexcept
{
    return ptrToBytes(blockSpace) - realOccupiedBlockMetaSize;
}

const void *blockAdressFromFreeSpace(const void *blockSpace) noexcept
{
    return ptrToBytes(blockSpace) - realOccupiedBlockMetaSize;
}

size_t fullBlockSize(const void *block) noexcept
{
    return realOccupiedBlockMetaSize + accessBlockSize(block);
}

void *nextBlockBySize(void *block) noexcept
{
    return ptrToBytes(block) + fullBlockSize(block);
}

const void *nextBlockBySize(const void *block) noexcept
{
    return ptrToBytes(block) + fullBlockSize(block);
}

// Блок занят этим аллокатором?

bool isKidOfAllocator(const void *block, const void *metaStart) noexcept
{
    return accessBlockOwner(block) == metaStart;
}

// Функции для всего огромного memory block'а

void *allocateMemoryBlock(size_t spaceSize, std::pmr::memory_resource *parentAllocator)
{
    if (parentAllocator != nullptr)
    {
        return parentAllocator->allocate(spaceSize, defaultAlign);
    }

    return ::operator new(spaceSize);
}

void releaseMemoryBlock(void *metaStart) noexcept
{
    if (metaStart == nullptr)
    {
        return;
    }

    auto *parentAllocator = accessParent(metaStart);
    size_t totalSize = accessTotalSize(metaStart);

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

void initializeMemoryBlock(void *metaStart, size_t spaceSize, std::pmr::memory_resource *parentAllocator, allocator_with_fit_mode::fit_mode fitMode)
{
    accessParent(metaStart) = parentAllocator;
    accessFitMode(metaStart) = fitMode;
    accessTotalSize(metaStart) = spaceSize;

    new (&accessMutex(metaStart)) std::mutex();

    accessFirstOccupiedBlock(metaStart) = nullptr;
}

size_t gapSize(const void *left, const void *right) noexcept
{
	return ptrToBytes(const_cast<void*>(right)) - ptrToBytes(const_cast<void*>(left));
}

bool canBlockFit(size_t gapSize, size_t neededSpaceSize) noexcept
{
	return gapSize >= realOccupiedBlockMetaSize + neededSpaceSize;
}

void *lastOccupiedBlock(void *metaStart) noexcept
{
    void *current = accessFirstOccupiedBlock(metaStart);

    if (current == nullptr)
    {
        return nullptr;
    }

    while (accessNextOccupied(current) != nullptr)
    {
        current = accessNextOccupied(current);
    }

    return current;
}

const void *lastOccupiedBlock(const void *metaStart) noexcept
{
    const void *current = accessFirstOccupiedBlock(metaStart);

    if (current == nullptr)
    {
        return nullptr;
    }

    while (accessNextOccupied(current) != nullptr)
    {
        current = accessNextOccupied(current);
    }

    return current;
}

void *freeRegionBegin(void *metaStart, void *blockOnRight) noexcept
{
    if (blockOnRight == nullptr)
    {
        void *last = lastOccupiedBlock(metaStart);

        if (last == nullptr)
        {
            return accessFirstBlock(metaStart);
        }

        return nextBlockBySize(last);
    }

    void *prev = accessPrevOccupied(blockOnRight);

    if (prev == nullptr)
    {
        return accessFirstBlock(metaStart);
    }

    return nextBlockBySize(prev);
}

const void *freeRegionBegin(const void *metaStart, const void *blockOnRight) noexcept
{
    if (blockOnRight == nullptr)
    {
        const void *last = lastOccupiedBlock(metaStart);

        if (last == nullptr)
        {
            return accessFirstBlock(metaStart);
        }

        return nextBlockBySize(last);
    }

    const void *prev = accessPrevOccupied(blockOnRight);

    if (prev == nullptr)
    {
        return accessFirstBlock(metaStart);
    }

    return nextBlockBySize(prev);
}

void *freeRegionEnd(void *metaStart, void *blockOnRight) noexcept
{
    if (blockOnRight == nullptr)
    {
        return accessEnd(metaStart);
    }

    return blockOnRight;
}

const void *freeRegionEnd(const void *metaStart, const void *blockOnRight) noexcept
{
    if (blockOnRight == nullptr)
    {
        return accessEnd(metaStart);
    }

    return blockOnRight;
}

bool hasFreeRegionBefore(void *metaStart, void *blockOnRight) noexcept
{
    return freeRegionBegin(metaStart, blockOnRight) < freeRegionEnd(metaStart, blockOnRight);
}

bool hasFreeRegionBefore(const void *metaStart, const void *blockOnRight) noexcept
{
    return freeRegionBegin(metaStart, blockOnRight) < freeRegionEnd(metaStart, blockOnRight);
}


// <<< Непосредственно задача >>>

allocator_boundary_tags::~allocator_boundary_tags()
{
    releaseMemoryBlock(_trusted_memory);
    _trusted_memory = nullptr;
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other)
    {
        releaseMemoryBlock(_trusted_memory);
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }

    return *this;
}


/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode) : _trusted_memory(nullptr)
{
    if (parent_allocator == nullptr)
    {
        parent_allocator = std::pmr::get_default_resource();
    }

    if (space_size < occupied_block_metadata_size)
    {
        throw std::bad_alloc();
    }

    const size_t totalSize = realAllocatorMetaSize + space_size;
    
    _trusted_memory = allocateMemoryBlock(totalSize, parent_allocator);

    try
    {
        initializeMemoryBlock(_trusted_memory, totalSize, parent_allocator, allocate_fit_mode);
    }
    catch (...)
    {
        parent_allocator->deallocate(_trusted_memory, totalSize, defaultAlign);
        _trusted_memory = nullptr;
        throw;
    }
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));

    auto mode = accessFitMode(_trusted_memory);

    void *theRightBlock = nullptr;
    size_t chosenSize = 0;
    bool found = false;

    for (void *right = accessFirstOccupiedBlock(_trusted_memory); ; )
    {
        void *currentStart = freeRegionBegin(_trusted_memory, right);
        void *currentEnd = freeRegionEnd(_trusted_memory, right);

        size_t currentSize = gapSize(currentStart, currentEnd);

        if (canBlockFit(currentSize, size))
        {
            if (!found)
            {
                found = true;
                theRightBlock = right;
                chosenSize = currentSize;

                if (mode == allocator_with_fit_mode::fit_mode::first_fit)
                {
                    break;
                }
            }
            else
            {
                if ((mode == allocator_with_fit_mode::fit_mode::the_best_fit &&
                    currentSize < chosenSize) ||
                    (mode == allocator_with_fit_mode::fit_mode::the_worst_fit &&
                    currentSize > chosenSize))
                {
                    theRightBlock = right;
                    chosenSize = currentSize;
                }
            }
        }

        if (right == nullptr)
        {
            break;
        }

        right = accessNextOccupied(right);


    }

    if (!found)
    {
        throw std::bad_alloc();
    }

    void *newBlock = freeRegionBegin(_trusted_memory, theRightBlock);

    void *prev = (theRightBlock == nullptr) ? lastOccupiedBlock(_trusted_memory) : accessPrevOccupied(theRightBlock);

    size_t usefulSize = size;
    size_t tail = chosenSize - (realOccupiedBlockMetaSize + size);

    if (tail < realOccupiedBlockMetaSize)
    {
        usefulSize = chosenSize - realOccupiedBlockMetaSize;
    }

    accessBlockSize(newBlock) = usefulSize;
    accessBlockOwner(newBlock) = _trusted_memory;
    accessPrevOccupied(newBlock) = prev;
    accessNextOccupied(newBlock) = theRightBlock;

    if (prev == nullptr)
    {
        accessFirstOccupiedBlock(_trusted_memory) = newBlock;
    }
    else
    {
        accessNextOccupied(prev) = newBlock;
    }

    if (theRightBlock != nullptr)
    {
        accessPrevOccupied(theRightBlock) = newBlock;
    }

    return accessBlockSpace(newBlock);
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    if (at == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));

    void *block = blockAdressFromFreeSpace(at);

    if (!isKidOfAllocator(block, _trusted_memory))
    {
        throw std::invalid_argument("Блок не принадлежит данному аллокатору!");
    }

    void *prev = accessPrevOccupied(block);
    void *next = accessNextOccupied(block);

    if (prev == nullptr)
    {
        accessFirstOccupiedBlock(_trusted_memory) = next;
    }
    else
    {
        accessNextOccupied(prev) = next;
    }

    if (next != nullptr)
    {
        accessPrevOccupied(next) = prev;
    }
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
    accessFitMode(_trusted_memory) = mode;
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
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

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return {};
    }

    boundary_iterator i;
    i.setTrusted(_trusted_memory);

    void *first = accessFirstOccupiedBlock(_trusted_memory);

    if (first == nullptr)
    {
        i.setOcupiedPtr(nullptr);
        i.setOccupied(false);
        return i;
    }

    if (accessFirstBlock(_trusted_memory) < first)
    {
        i.setOcupiedPtr(first);
        i.setOccupied(false);
    }
    else
    {
        i.setOcupiedPtr(first);
        i.setOccupied(false);
    }

    return i;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    boundary_iterator i;
    i.setter(_trusted_memory, nullptr ? nullptr : accessEnd(_trusted_memory), false);
    return i;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> result;

    void *current = accessFirstOccupiedBlock(_trusted_memory);
    void *ptr = accessFirstBlock(_trusted_memory);

    if (current == nullptr)
    {
        size_t freeSize = gapSize(ptr, accessEnd(_trusted_memory));

        if (freeSize > 0)
        {
            result.push_back({
                .block_size = freeSize,
                .is_block_occupied = false
            });
        }

        return result;
    }

    while (current != nullptr)
    {
        if (ptr < current)
        {
            result.push_back({
                .block_size = gapSize(ptr, current),
                .is_block_occupied = false
            });
        }

        result.push_back({
            .block_size = fullBlockSize(current),
            .is_block_occupied = true
        });

        ptr = nextBlockBySize(current);
        current = accessNextOccupied(current);
    }

    if (ptr < accessEnd(_trusted_memory))
    {
        result.push_back({
            .block_size = gapSize(ptr, accessEnd(_trusted_memory)),
            .is_block_occupied = false
        });
    }

    return result;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other) : _trusted_memory(nullptr)
{
    if (other._trusted_memory == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(accessMutex(other._trusted_memory));

    size_t spaceSize = accessTotalSize(other._trusted_memory);
    auto *parentAllocator = accessParent(other._trusted_memory);
    auto fitMode = accessFitMode(other._trusted_memory);

    _trusted_memory = allocateMemoryBlock(spaceSize, parentAllocator);
    initializeMemoryBlock(_trusted_memory, spaceSize, parentAllocator, fitMode);

    accessMutex(_trusted_memory).~mutex();
    std::memcpy(_trusted_memory, other._trusted_memory, spaceSize);
    new (&accessMutex(_trusted_memory)) std::mutex();
    
    // Указатель fix
    std::ptrdiff_t delta = ptrToBytes(_trusted_memory) - ptrToBytes(other._trusted_memory);

    if (accessFirstOccupiedBlock(_trusted_memory) != nullptr)
    {
        accessFirstOccupiedBlock(_trusted_memory) =
            ptrToBytes(accessFirstOccupiedBlock(_trusted_memory)) + delta;
    }

    for (void *block = accessFirstOccupiedBlock(_trusted_memory); block != nullptr; block = accessNextOccupied(block))
    {
        accessBlockOwner(block) = _trusted_memory;

        if (accessPrevOccupied(block) != nullptr)
        {
            accessPrevOccupied(block) = ptrToBytes(accessPrevOccupied(block)) + delta;
        }

        if (accessNextOccupied(block) != nullptr)
        {
            accessNextOccupied(block) = ptrToBytes(accessNextOccupied(block)) + delta;
        }
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this == &other)
    {
        return *this;
    }

    allocator_boundary_tags temp(other);
    *this = std::move(temp);

    return *this;
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr && _occupied == other._occupied && _trusted_memory == other._trusted_memory;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (_trusted_memory == nullptr)
    {
        return *this;
    }

    void *endPtr = accessEnd(_trusted_memory);

    if (_occupied_ptr == endPtr && !_occupied)
    {
        return *this;
    }

    if (_occupied)
    {
        void *next = accessNextOccupied(_occupied_ptr);

        if (next != nullptr)
        {
            if (nextBlockBySize(_occupied_ptr) < next)
            {
                _occupied_ptr = next;
                _occupied = false;
            }
            else
            {
                _occupied_ptr = next;
                _occupied = true;
            }
        }
        else
        {
            if (nextBlockBySize(_occupied_ptr) < accessEnd(_trusted_memory))
            {
                _occupied_ptr = nullptr;
                _occupied = false;
            }
            else
            {
                _occupied_ptr = endPtr;
                _occupied = false;
            }
        }
    }
    else
    {
        if (_occupied_ptr == nullptr)
        {
            _occupied_ptr = endPtr;
            _occupied = false;
        }
        else
        {
            _occupied = true;
        }
    }

    return *this;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (_trusted_memory == nullptr)
    {
        return *this;
    }

    void *endPtr = accessEnd(_trusted_memory);

    if (_occupied_ptr == endPtr && !_occupied)
    {
        void *last = lastOccupiedBlock(_trusted_memory);

        if (last == nullptr)
        {
            _occupied_ptr = nullptr;
            _occupied = false;
            return *this;
        }

        if (nextBlockBySize(last) < accessEnd(_trusted_memory))
        {
            _occupied_ptr = nullptr;
            _occupied = false;
        }
        else
        {
            _occupied_ptr = last;
            _occupied = true;
        }

        return *this;
    }

    if (_occupied)
    {
        void *prev = accessPrevOccupied(_occupied_ptr);

        if (prev != nullptr)
        {
            if (nextBlockBySize(prev) < _occupied_ptr)
            {
                _occupied = false;
            }
            else
            {
                _occupied_ptr = prev;
                _occupied = true;
            }
        }
        else
        {
            if (accessFirstBlock(_trusted_memory) < _occupied_ptr)
            {
                _occupied = false;
            }
        }
    }
    else
    {
        if (_occupied_ptr == nullptr)
        {
            void *last = lastOccupiedBlock(_trusted_memory);

            if (last != nullptr)
            {
                _occupied_ptr = last;
                _occupied = true;
            }
        }
        else
        {
            void *prev = accessPrevOccupied(_occupied_ptr);

            if (prev != nullptr)
            {
                _occupied_ptr = prev;
                _occupied = true;
            }
        }
    }

    return *this;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    auto copy = *this;
    ++(*this);
    return copy;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    auto copy = *this;
    --(*this);
    return copy;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return 0;
    }

    void *endPtr = accessEnd(_trusted_memory);

    if (_occupied_ptr == endPtr && !_occupied)
    {
        return 0;
    }

    if (_occupied)
    {
        return fullBlockSize(_occupied_ptr);
    }

    void *left = nullptr;
    void *right = nullptr;

    if (_occupied_ptr == nullptr)
    {
        left = freeRegionBegin(_trusted_memory, nullptr);
        right = accessEnd(_trusted_memory);
    }
    else
    {
        left = freeRegionBegin(_trusted_memory, _occupied_ptr);
        right = _occupied_ptr;
    }

    return gapSize(left, right);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    return get_ptr();
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
: _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr)
{
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
: _occupied_ptr(nullptr), _occupied(false), _trusted_memory(trusted)
{
    if (_trusted_memory == nullptr)
    {
        return;
    }

    void *first = accessFirstOccupiedBlock(_trusted_memory);

    if (first == nullptr)
    {
        _occupied_ptr = nullptr;
        _occupied = false;
        return;
    }

    if (accessFirstBlock(_trusted_memory) < first)
    {
        _occupied_ptr = first;
        _occupied = false;
    }
    else
    {
        _occupied_ptr = first;
        _occupied = true;
    }
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }

    if (_occupied)
    {
        return _occupied_ptr;
    }

    if (_occupied_ptr == accessEnd(_trusted_memory))
    {
        return nullptr;
    }

    if (_occupied_ptr == nullptr)
    {
        return freeRegionBegin(_trusted_memory, nullptr);
    }

    return freeRegionBegin(_trusted_memory, _occupied_ptr);
}