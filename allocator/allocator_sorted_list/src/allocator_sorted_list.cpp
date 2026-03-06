#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"

// Align.

constexpr size_t alignUp(size_t value, size_t alignment) noexcept
{
    return (value + alignment - 1) / alignment * alignment;
}

constexpr size_t defaultAlign = alignof(std::max_align_t);

// Оффсеты - метаданные аллокатора
// [Родительский аллокатор][ФитМод][Размер][Мьютекс][ПервыйСвобод.]

constexpr size_t parentAllocatorOffset = 0;

constexpr size_t fitModeOffset = alignUp(parentAllocatorOffset + sizeof(std::pmr::memory_resource *), alignof(allocator_with_fit_mode::fit_mode));

constexpr size_t totalSizeOffset = alignUp(fitModeOffset + sizeof(allocator_with_fit_mode::fit_mode), alignof(size_t));

constexpr size_t mutexOffset = alignUp(totalSizeOffset + sizeof(size_t), alignof(std::mutex));

constexpr size_t firstFreeBlockOffset = alignUp(mutexOffset + sizeof(std::mutex), alignof(void *));

constexpr size_t realAllocatorMetaSize = alignUp(firstFreeBlockOffset + sizeof(void *), defaultAlign);

// Оффсеты - блок
// [Следующий свободный / Метаданные аллокатора][Размер]

constexpr size_t blockFirstFieldOffset = 0;

constexpr size_t blockSizeOffset = alignUp(blockFirstFieldOffset + sizeof(void *), alignof(size_t));

constexpr size_t realBlockMetaSize = alignUp(blockSizeOffset + sizeof(size_t), defaultAlign);

// Минимальный размер полезной нагрузки

constexpr size_t minimalFreeSize = defaultAlign;

// Конвертация укзателя в байтовый

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
T &accessField(void *base, size_t offset) {
    return *reinterpret_cast<T*>(ptrToBytes(base) + offset);
}

template<typename T>
T &accessField(const void *base, size_t offset) {
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

void *&accessFirstFreeBlock(void *metaStart) noexcept
{
    return accessField<void*>(metaStart, firstFreeBlockOffset);
}

const void *accessFirstFreeBlock(const void *metaStart) noexcept
{
    return accessField<const void*>(metaStart, firstFreeBlockOffset);
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

void *&accessFirstBlockField(void *block) noexcept
{
    return accessField<void*>(block, blockFirstFieldOffset);
}

const void *accessFirstBlockField(const void *block) noexcept
{
    return accessField<const void*>(block, blockFirstFieldOffset);
}

size_t &accessBlockSize(void *block) noexcept
{
    return accessField<size_t>(block, blockSizeOffset);
}

size_t accessBlockSize(const void *block) noexcept
{
    return accessField<size_t>(block, blockSizeOffset);
}

void *accessBlockSpace(void *block) noexcept
{
    return ptrToBytes(block) + realBlockMetaSize;
}

const void *accessBlockSpace(const void *block) noexcept
{
    return ptrToBytes(block) + realBlockMetaSize;
}

void *blockAdressFromFreeSpace(void *blockSpace) noexcept
{
    return ptrToBytes(blockSpace) - realBlockMetaSize;
}

const void *blockAdressFromFreeSpace(const void *blockSpace) noexcept
{
    return ptrToBytes(blockSpace) - realBlockMetaSize;
}

size_t fullBlockSize(const void *block) noexcept
{
    return realBlockMetaSize + accessBlockSize(block);
}

void *nextBlock(void *block) noexcept
{
    return ptrToBytes(block) + fullBlockSize(block);
}

const void *nextBlock(const void *block) noexcept
{
    return ptrToBytes(block) + fullBlockSize(block);
}

// Блок занят этим аллокатором?

bool isKidOfAllocator(const void *block, const void *metaStart) noexcept
{
    return accessFirstBlockField(block) == metaStart;
}

// Два блока соседние?

bool areNeighbors(const void *left, const void *right) noexcept
{
    return nextBlock(left) == right;
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

inline void initializeMemoryBlock(void *metaStart, size_t spaceSize, std::pmr::memory_resource *parentAllocator, allocator_with_fit_mode::fit_mode mode)
{
	accessParent(metaStart) = parentAllocator;
	accessFitMode(metaStart) = mode;
	accessTotalSize(metaStart) = spaceSize;

	new (&accessMutex(metaStart)) std::mutex();

	void *initialBlock = accessFirstBlock(metaStart);

	accessFirstFreeBlock(metaStart) = initialBlock;

	accessFirstBlockField(initialBlock) = nullptr;

	accessBlockSize(initialBlock) = spaceSize - realAllocatorMetaSize - realBlockMetaSize;
}


// <<< Непосредственно задача >>>

// Деструктор
allocator_sorted_list::~allocator_sorted_list()
{
    releaseMemoryBlock(_trusted_memory);
    _trusted_memory = nullptr;
}

// Конструктор перемещением
allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
    : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
}

// Присваивание перемещением
allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    if (this != &other) {
        releaseMemoryBlock(_trusted_memory);
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }

    return *this;
}

// Конструктор
allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < realAllocatorMetaSize + realBlockMetaSize + minimalFreeSize)
    {
        throw std::bad_alloc();
    }

    _trusted_memory = allocateMemoryBlock(space_size, parent_allocator);

    try
    {
        initializeMemoryBlock(_trusted_memory, space_size, parent_allocator, allocate_fit_mode);
    }
    catch (...)
    {
        if (parent_allocator != nullptr)
        {
            parent_allocator->deallocate(_trusted_memory, space_size, defaultAlign);
        }
        else
        {
            ::operator delete(_trusted_memory);
        }

        _trusted_memory = nullptr;
        throw;
    }
}

// Аллокатор!
[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(
    size_t size)
{
	if (size == 0)
	{
		return nullptr;
	}
	
	std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
	
	size_t fixedSize = alignUp(size, defaultAlign);
	
	void *selectedBlock = nullptr;
	void *selectedPrev = nullptr;
	
	void *prev = nullptr;
	void *current = accessFirstFreeBlock(_trusted_memory);
	
	switch (accessFitMode(_trusted_memory))
	{
		case fit_mode::first_fit:
		{
			while (current != nullptr)
			{
				if (accessBlockSize(current) >= fixedSize)
				{
					selectedBlock = current;
					selectedPrev = prev;
					break;
				}
	
				prev = current;
				current = accessFirstBlockField(current);
			}
	
			break;
		}
	
		case fit_mode::the_best_fit:
		{
			size_t best_size = static_cast<size_t>(-1);
	
			while (current != nullptr)
			{
				size_t current_size = accessBlockSize(current);
	
				if (current_size >= fixedSize && current_size < best_size)
				{
					best_size = current_size;
					selectedBlock = current;
					selectedPrev = prev;
				}
	
				prev = current;
				current = accessFirstBlockField(current);
			}
	
			break;
		}
	
		case fit_mode::the_worst_fit:
		{
			size_t worst_size = 0;
	
			while (current != nullptr)
			{
				size_t current_size = accessBlockSize(current);
	
				if (current_size >= fixedSize && current_size > worst_size)
				{
					worst_size = current_size;
					selectedBlock = current;
					selectedPrev = prev;
				}
	
				prev = current;
				current = accessFirstBlockField(current);
			}
	
			break;
		}
	}
	
	if (selectedBlock == nullptr)
	{
		throw std::bad_alloc();
	}
	
	void *nextFree = accessFirstBlockField(selectedBlock);
	size_t oldBlockSpaceSize = accessBlockSize(selectedBlock);
	
	bool couldSplit =
		oldBlockSpaceSize >= fixedSize + realBlockMetaSize + minimalFreeSize;
	
	if (couldSplit)
	{
		void *newFreeBlock = ptrToBytes(selectedBlock) + realBlockMetaSize + fixedSize;
	
		size_t newFreeBlockSpaceSize = oldBlockSpaceSize - fixedSize - realBlockMetaSize;
	
		accessFirstBlockField(newFreeBlock) = nextFree;
		accessBlockSize(newFreeBlock) = newFreeBlockSpaceSize;
	
		if (selectedPrev == nullptr)
		{
			accessFirstFreeBlock(_trusted_memory) = newFreeBlock;
		}
		else
		{
			accessFirstBlockField(selectedPrev) = newFreeBlock;
		}
	
		accessFirstBlockField(selectedBlock) = _trusted_memory;
		accessBlockSize(selectedBlock) = fixedSize;
	}
	else
	{
		if (selectedPrev == nullptr)
		{
			accessFirstFreeBlock(_trusted_memory) = nextFree;
		}
		else
		{
			accessFirstBlockField(selectedPrev) = nextFree;
		}
		accessFirstBlockField(selectedBlock) = _trusted_memory;
	}
	
	return accessBlockSpace(selectedBlock);
}


// Конструктор копированием
allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other) : _trusted_memory(nullptr)
{
	if (other._trusted_memory == nullptr)
	{
		return;
	}
	
	std::lock_guard<std::mutex> lock(accessMutex(other._trusted_memory));
	
	size_t spaceSize = accessTotalSize(other._trusted_memory);
	auto *parentAllocator = accessParent(other._trusted_memory);
	auto mode = accessFitMode(other._trusted_memory);
	
	_trusted_memory = allocateMemoryBlock(spaceSize, parentAllocator);
	initializeMemoryBlock(_trusted_memory, spaceSize, parentAllocator, mode);

	accessMutex(_trusted_memory).~mutex();
	
	std::memcpy(_trusted_memory, other._trusted_memory, spaceSize);
	
	new (&accessMutex(_trusted_memory)) std::mutex();
	
	// Указатели fix
	std::ptrdiff_t delta = ptrToBytes(_trusted_memory) - ptrToBytes(other._trusted_memory);
	
	if (accessFirstFreeBlock(_trusted_memory) != nullptr)
	{
		accessFirstFreeBlock(_trusted_memory) = ptrToBytes(accessFirstFreeBlock(_trusted_memory)) + delta;
	}
	
	for (void *block = accessFirstBlock(_trusted_memory);
		 ptrToBytes(block) < ptrToBytes(accessEnd(_trusted_memory));
		 block = nextBlock(block))
	{
		if (accessFirstBlockField(block) == other._trusted_memory)
		{
			accessFirstBlockField(block) = _trusted_memory;
		}
		else if (accessFirstBlockField(block) != nullptr)
		{
			accessFirstBlockField(block) = ptrToBytes(accessFirstBlockField(block)) + delta;
		}
	}
}

// Присваивание копированием
allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other)
{
    if (this == &other)
    {
        return *this;
    }

    allocator_sorted_list temp(other);
    *this = std::move(temp);

    return *this;
}

// Равенство
bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

void allocator_sorted_list::do_deallocate_sm(
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
	
	void *prevFree = nullptr;
	void *currFree = accessFirstFreeBlock(_trusted_memory);
	
	while (currFree != nullptr && currFree < block)
	{
		prevFree = currFree;
		currFree = accessFirstBlockField(currFree);
	}
	
	accessFirstBlockField(block) = currFree;
	
	if (prevFree == nullptr)
	{
		accessFirstFreeBlock(_trusted_memory) = block;
	}
	else
	{
		accessFirstBlockField(prevFree) = block;
	}
	
	if (currFree != nullptr && areNeighbors(block, currFree))
	{
		accessBlockSize(block) += realBlockMetaSize + accessBlockSize(currFree);
	
		accessFirstBlockField(block) = accessFirstBlockField(currFree);
		currFree = accessFirstBlockField(block);
	}
	
	if (prevFree != nullptr && areNeighbors(prevFree, block))
	{
		accessBlockSize(prevFree) += realBlockMetaSize + accessBlockSize(block);
	
		accessFirstBlockField(prevFree) = accessFirstBlockField(block);
	}
}



inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
    accessFitMode(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
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


std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const
{
	std::vector<allocator_test_utils::block_info> result;
	
	for (auto i = begin(); i != end(); ++i)
	{
		result.push_back({ i.size(), i.occupied() });
	}
	
	return result;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept
{
    if (_trusted_memory == nullptr)
	{
		return {};
	}
	
	return sorted_free_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    return {};
}


bool allocator_sorted_list::sorted_free_iterator::operator==(
        const allocator_sorted_list::sorted_free_iterator & other) const noexcept
{
    return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
        const allocator_sorted_list::sorted_free_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept
{
    if (_free_ptr != nullptr)
    {
        _free_ptr = accessFirstBlockField(_free_ptr);
    }

    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    auto copy = *this;
    ++(*this);
    return copy;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    if (_free_ptr == nullptr)
    {
        return 0;
    }

    return accessBlockSize(_free_ptr);
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
{
    return _free_ptr;
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator() : _free_ptr(nullptr)
{
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *trusted)
: _free_ptr(trusted == nullptr ? nullptr : accessFirstFreeBlock(trusted))
{
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    if (_trusted_memory == nullptr)
    {
        return {};
    }

    return sorted_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    sorted_iterator i;
	i.setEnd(_trusted_memory, const_cast<void*>(accessEnd(_trusted_memory)));
	return i;
}

bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator & other) const noexcept
{
    return _current_ptr == other._current_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept
{
    if (_current_ptr != nullptr && _trusted_memory != nullptr)
	{
		void *next = nextBlock(_current_ptr);
	
		if (next >= accessEnd(_trusted_memory))
		{
			_current_ptr = const_cast<void*>(accessEnd(_trusted_memory));
		}
		else
		{
			_current_ptr = next;
		}
	}
	
	return *this;
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int n)
{
    auto copy = *this;
    ++(*this);
    return copy;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept
{
    if (_current_ptr == nullptr || _trusted_memory == nullptr)
    {
        return 0;
    }

    if (_current_ptr == accessEnd(_trusted_memory))
    {
        return 0;
    }

    return accessBlockSize(_current_ptr);
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept
{
    return _current_ptr;
}

allocator_sorted_list::sorted_iterator::sorted_iterator()
: _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(nullptr)
{
}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted)\
: _free_ptr(trusted == nullptr ? nullptr : accessFirstFreeBlock(trusted)),
_current_ptr(trusted == nullptr ? nullptr : accessFirstBlock(trusted)),
_trusted_memory(trusted)
{
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
	if (_current_ptr == nullptr || _trusted_memory == nullptr)
	{
		return false;
	}
	
	if (_current_ptr == accessEnd(_trusted_memory))
	{
		return false;
	}
	
	return isKidOfAllocator(_current_ptr, _trusted_memory);
}
