#include <not_implemented.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "../include/allocator_red_black_tree.h"

// Тесты запускаются из корня:
// ./build/allocator/allocator_red_black_tree/tests/sys_prog_allctr_allctr_rb_tr_tests

#pragma region HelperFuncs

#pragma region Offsets

// Аллокатор MetaData
// [Родит.Аллокатор*][фитмод][размер выдел. памяти][мьютекс][корень*]

constexpr size_t parentOffset = 0;

constexpr size_t fitModeOffset = sizeof(void*);

constexpr size_t totalSizeOffset = fitModeOffset + sizeof(allocator_with_fit_mode::fit_mode);

constexpr size_t mutexOffset = totalSizeOffset + sizeof(size_t);

constexpr size_t rootOffset = mutexOffset + sizeof(std::mutex);

// Занятого MetaData
// [занят/Свободен + Цвет (data)][предыдущий*][следующий*][метаданные аллокатора*]

// Свободного MetaData
// [data][предыдущий*][следующий*][левое поддерево*][правое поддерево*][метаданные аллокатора*]

constexpr size_t blockDataOffset = 0;

constexpr size_t prevBlockOffset = sizeof(allocator_red_black_tree::block_data);

constexpr size_t nextBlockOffset = prevBlockOffset + sizeof(void*);

constexpr size_t occupiedParentOffset = nextBlockOffset + sizeof(void*);

constexpr size_t freeLeftOffset = nextBlockOffset + sizeof(void*);

constexpr size_t freeRightOffset = freeLeftOffset + sizeof(void*);

constexpr size_t freeParentOffset = freeRightOffset + sizeof(void*);

#pragma endregion

#pragma region Accesses

// Конвертация указателя в байтовый

template <typename T>
T* ptrToBytes(void* ptr, ptrdiff_t offset) noexcept
{
    return reinterpret_cast<T*>(static_cast<std::byte*>(ptr) + offset);
}

// Получить поле из offset

template <typename T>
T& accessField(void* base, size_t offset) noexcept
{
    return *ptrToBytes<T>(base, offset);
}

// Доступ к аллокатору

void* &accessRoot(void* metaStart)
{
	return accessField<void*>(metaStart, rootOffset);
}

std::mutex& accessMutex(void* metaStart)
{
	return accessField<std::mutex>(metaStart, mutexOffset);
}

allocator_with_fit_mode::fit_mode& accessFitMode(void* metaStart)
{
	return accessField<allocator_with_fit_mode::fit_mode>(metaStart, fitModeOffset);
}

size_t accessTotalSize(void* metaStart)
{
	return accessField<size_t>(metaStart, totalSizeOffset);
}

// Доступ к блоку

allocator_red_black_tree::block_data& accessBlockData(void* block)
{
	return accessField<allocator_red_black_tree::block_data>(block, blockDataOffset);
}

void* &accessPrevBlock(void* block)
{
	return accessField<void*>(block, prevBlockOffset);
}

void* &accessNextBlock(void* block)
{
	return accessField<void*>(block, nextBlockOffset);
}

void* &accessLeft(void* block)
{
	return accessField<void*>(block, freeLeftOffset);
}

void* &accessRight(void* block)
{
	return accessField<void*>(block, freeRightOffset);
}

void* &accessParent(void* block)
{ 
    return accessBlockData(block).occupied ? accessField<void*>(block, occupiedParentOffset) : accessField<void*>(block, freeParentOffset); 
}

size_t accessBlockFullSize(void* block, void* metaStart) 
{
    void* next = accessNextBlock(block);
    if (next == nullptr) 
        return static_cast<size_t>(static_cast<std::byte*>(metaStart) + accessTotalSize(metaStart) - static_cast<std::byte*>(block));
    return static_cast<size_t>(static_cast<std::byte*>(next) - static_cast<std::byte*>(block));
}

#pragma endregion

#pragma region Tree

void rotateLeft(void* metaStart, void* x)
{
    void* y = accessRight(x);
    accessRight(x) = accessLeft(y);
    
    if (accessLeft(y) != nullptr)
    {
	    accessParent(accessLeft(y)) = x;
	}
	
    accessParent(y) = accessParent(x);
    
    if (accessParent(x) == nullptr)
    {
	    accessRoot(metaStart) = y;
	}
	else if (x == accessLeft(accessParent(x)))
    {
	    accessLeft(accessParent(x)) = y;
	}
	else
    {
	    accessRight(accessParent(x)) = y;
	}
	
    accessLeft(y) = x;
    accessParent(x) = y;
}

void rotateRight(void* metaStart, void* y)
{
    void* x = accessLeft(y);
    accessLeft(y) = accessRight(x);
    
    if (accessRight(x) != nullptr)
    {
	    accessParent(accessRight(x)) = y;
	}
	
    accessParent(x) = accessParent(y);
    
    if (accessParent(y) == nullptr)
    {
	    accessRoot(metaStart) = x;
	}
    else if (y == accessLeft(accessParent(y)))
    {
	    accessLeft(accessParent(y)) = x;
	}
    else
    {
	    accessRight(accessParent(y)) = x;
	}
	
    accessRight(x) = y;
    accessParent(y) = x;
}

void fixInsertion(void* metaStart, void* x)
{
    while (accessParent(x) != nullptr && 
           accessBlockData(accessParent(x)).color == allocator_red_black_tree::block_color::RED)
    {
        void* parent = accessParent(x);
        void* grandparent = accessParent(parent);
        
        if (parent == accessLeft(grandparent))
        {
            void* uncle = accessRight(grandparent);
            
            // 1. Red Дядя
            if (uncle && accessBlockData(uncle).color == allocator_red_black_tree::block_color::RED)
            {
                accessBlockData(parent).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(uncle).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(grandparent).color = allocator_red_black_tree::block_color::RED;
                
                x = grandparent;
            }
            else
            {
                // 2. LR
                if (x == accessRight(parent))
                {
                    x = parent;
                    rotateLeft(metaStart, x);
                    parent = accessParent(x);
                    grandparent = accessParent(parent);
                }
                
                // 3. LL
                accessBlockData(parent).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(grandparent).color = allocator_red_black_tree::block_color::RED;
                rotateRight(metaStart, grandparent);
            }
        }
        else
        {
            void* uncle = accessLeft(grandparent);
            
            // 1. Red Дядя
            if (uncle && accessBlockData(uncle).color == allocator_red_black_tree::block_color::RED)
            {
                accessBlockData(parent).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(uncle).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(grandparent).color = allocator_red_black_tree::block_color::RED;
                
                x = grandparent;
            }
            else
            {
	            // 2. LR
                if (x == accessLeft(parent))
                {
                    x = parent;
                    rotateRight(metaStart, x);
                    parent = accessParent(x);
                    grandparent = accessParent(parent);
                }
                
                // 3. LL
                accessBlockData(parent).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(grandparent).color = allocator_red_black_tree::block_color::RED;
                rotateLeft(metaStart, grandparent);
            }
        }
    }
    
    accessBlockData(accessRoot(metaStart)).color = allocator_red_black_tree::block_color::BLACK;
}

void transplant(void* metaStart, void* old, void* newElement)
{
    if (accessParent(old) == nullptr)
    {
        accessRoot(metaStart) = newElement;
    }
    else if (old == accessLeft(accessParent(old)))
    {
    
        accessLeft(accessParent(old)) = newElement;
    }
    else
    {
        accessRight(accessParent(old)) = newElement;
    }
    
    if (newElement)
    {
        accessParent(newElement) = accessParent(old);
    }
}

void fixDeletion(void* metaStart, void* x, void* xParent)
{
    while (x != accessRoot(metaStart) && 
           (x == nullptr || accessBlockData(x).color == allocator_red_black_tree::block_color::BLACK))
    {
        if (x == accessLeft(xParent))
        {
            void* brother = accessRight(xParent);
            
            // - - - Red брат - - -
            
            if (accessBlockData(brother).color == allocator_red_black_tree::block_color::RED)
            {
                accessBlockData(brother).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(xParent).color = allocator_red_black_tree::block_color::RED;
                rotateLeft(metaStart, xParent);
                brother = accessRight(xParent);
            }
            
            // - - - Black брат - - -
            
            // Black племянники
            if ((!accessLeft(brother) || accessBlockData(accessLeft(brother)).color == allocator_red_black_tree::block_color::BLACK) &&
                (!accessRight(brother) || accessBlockData(accessRight(brother)).color == allocator_red_black_tree::block_color::BLACK))
            {
                accessBlockData(brother).color = allocator_red_black_tree::block_color::RED;
                
                x = xParent;
                xParent = accessParent(x);
            }
            else
            {
                // Ближний племянник красный, а другой - чёрный
                if (!accessRight(brother) || 
                    accessBlockData(accessRight(brother)).color == allocator_red_black_tree::block_color::BLACK)
                {
                    if (accessLeft(brother))
                    {
	                    accessBlockData(accessLeft(brother)).color = allocator_red_black_tree::block_color::BLACK;
                    }
                    accessBlockData(brother).color = allocator_red_black_tree::block_color::RED;
                    
                    rotateRight(metaStart, brother);
                    brother = accessRight(xParent);
                }
                
                // Дальний племянник - красный
                accessBlockData(brother).color = accessBlockData(xParent).color;
                accessBlockData(xParent).color = allocator_red_black_tree::block_color::BLACK;
                
                if (accessRight(brother))
                {
                    accessBlockData(accessRight(brother)).color = allocator_red_black_tree::block_color::BLACK;
                }
                rotateLeft(metaStart, xParent);
                
                x = accessRoot(metaStart);
            }
        }
        else
        {
            void* brother = accessLeft(xParent);
            
            // - - - Red брат - - -
            
            if (accessBlockData(brother).color == allocator_red_black_tree::block_color::RED)
            {
                accessBlockData(brother).color = allocator_red_black_tree::block_color::BLACK;
                accessBlockData(xParent).color = allocator_red_black_tree::block_color::RED;
                rotateRight(metaStart, xParent);
                brother = accessLeft(xParent);
            }
            
            // - - - Black брат - - -
            
            // Black племянники
            if ((!accessRight(brother) || accessBlockData(accessRight(brother)).color == allocator_red_black_tree::block_color::BLACK) &&
                (!accessLeft(brother) || accessBlockData(accessLeft(brother)).color == allocator_red_black_tree::block_color::BLACK))
            {
                accessBlockData(brother).color = allocator_red_black_tree::block_color::RED;
                
                x = xParent;
                xParent = accessParent(x);
            }
            
            else
            {
                // Ближний племянник красный, а другой - чёрный
                if (!accessLeft(brother) || 
                    accessBlockData(accessLeft(brother)).color == allocator_red_black_tree::block_color::BLACK)
                {
                    if (accessRight(brother))
                    {
                        accessBlockData(accessRight(brother)).color = allocator_red_black_tree::block_color::BLACK;
                    }
                    accessBlockData(brother).color = allocator_red_black_tree::block_color::RED;
                    
                    rotateLeft(metaStart, brother);
                    brother = accessLeft(xParent);
                }
                
                // Дальний племянник - красный
                accessBlockData(brother).color = accessBlockData(xParent).color;
                accessBlockData(xParent).color = allocator_red_black_tree::block_color::BLACK;
                if (accessLeft(brother))
                {
	                accessBlockData(accessLeft(brother)).color = allocator_red_black_tree::block_color::BLACK;
                }
                rotateRight(metaStart, xParent);
                
                x = accessRoot(metaStart);
            }
        }
    }
    
    if (x)
    {
	    accessBlockData(x).color = allocator_red_black_tree::block_color::BLACK;
    }
}

void insertIntoTree(void* metaStart, void* node)
{
    accessBlockData(node).occupied = false;
    accessBlockData(node).color = allocator_red_black_tree::block_color::RED;
    accessLeft(node) = accessRight(node) = accessParent(node) = nullptr;
    
    void* root = accessRoot(metaStart);
    if (!root)
    {
        accessRoot(metaStart) = node;
        accessBlockData(node).color = allocator_red_black_tree::block_color::BLACK;
        return;
    }
    
    size_t nodeSize = accessBlockFullSize(node, metaStart);
    void* current = root;
    void* parent = nullptr;
    
    while (current)
    {
        parent = current;
        size_t currentSize = accessBlockFullSize(current, metaStart);
        
        if (nodeSize < currentSize || 
            (nodeSize == currentSize && node < current))
        {
            current = accessLeft(current);
        }
        else
        {
            current = accessRight(current);
        }
    }
    
    accessParent(node) = parent;
    if (nodeSize < accessBlockFullSize(parent, metaStart) || 
        (nodeSize == accessBlockFullSize(parent, metaStart) && node < parent))
    {
        accessLeft(parent) = node;
    }
    else
    {
        accessRight(parent) = node;
    }
    
    fixInsertion(metaStart, node);
}

void removeFromTree(void* metaStart, void* z)
{
    void* y = z;
    void* x;
    void* xParent;
    
    auto yColor = accessBlockData(y).color;
    
    if (!accessLeft(z))
    {
        x = accessRight(z);
        xParent = accessParent(z);
        transplant(metaStart, z, x);
    }
    else if (!accessRight(z))
    {
        x = accessLeft(z);
        xParent = accessParent(z);
        transplant(metaStart, z, x);
    }
    else
    {
        y = accessRight(z);
        while (accessLeft(y))
        {
            y = accessLeft(y);
        }
        
        yColor = accessBlockData(y).color;
        x = accessRight(y);
        
        if (accessParent(y) == z)
        {
            xParent = y;
        }
        else
        {
            xParent = accessParent(y);
            transplant(metaStart, y, x);
            accessRight(y) = accessRight(z);
            accessParent(accessRight(y)) = y;
        }
        
        transplant(metaStart, z, y);
        accessLeft(y) = accessLeft(z);
        accessParent(accessLeft(y)) = y;
        accessBlockData(y).color = accessBlockData(z).color;
    }
    
    if (yColor == allocator_red_black_tree::block_color::BLACK)
    {
        fixDeletion(metaStart, x, xParent);
    }
}

#pragma endregion

#pragma region TaskArea

allocator_red_black_tree::~allocator_red_black_tree()
{
    if (_trusted_memory)
    {
        accessMutex(_trusted_memory).~mutex();
        size_t total = accessTotalSize(_trusted_memory);
        auto* parent = accessField<std::pmr::memory_resource*>(_trusted_memory, parentOffset);
        
        if (parent)
        {
	        parent->deallocate(_trusted_memory, total);
	    }
        else
        {
	        ::operator delete(_trusted_memory);
        }
    }
}

allocator_red_black_tree::allocator_red_black_tree(
    allocator_red_black_tree &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_red_black_tree &allocator_red_black_tree::operator=(
    allocator_red_black_tree &&other) noexcept
{
    if (this != &other) {
        this->~allocator_red_black_tree();
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    return *this;    
}

allocator_red_black_tree::allocator_red_black_tree(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    size_t total = space_size + allocator_metadata_size;
    _trusted_memory = (parent_allocator) ? parent_allocator->allocate(total) : ::operator new(total);

    accessField<std::pmr::memory_resource*>(_trusted_memory, parentOffset) = parent_allocator;
    accessFitMode(_trusted_memory) = allocate_fit_mode;
    accessField<size_t>(_trusted_memory, totalSizeOffset) = total;
    new (&accessMutex(_trusted_memory)) std::mutex();
    accessRoot(_trusted_memory) = nullptr;

    void* first = ptrToBytes<void>(_trusted_memory, allocator_metadata_size);
    accessPrevBlock(first) = accessNextBlock(first) = nullptr;
    insertIntoTree(_trusted_memory, first);
}

allocator_red_black_tree::allocator_red_black_tree(const allocator_red_black_tree &other)
{
    if (other._trusted_memory == nullptr)
    {
        _trusted_memory = nullptr;
        return;
    }

    size_t total_size = accessTotalSize(other._trusted_memory);
    auto *parent = accessField<std::pmr::memory_resource*>(other._trusted_memory, parentOffset);
    _trusted_memory = (parent) ? parent->allocate(total_size) : ::operator new(total_size);
    std::memcpy(_trusted_memory, other._trusted_memory, total_size);
    new (&accessMutex(_trusted_memory)) std::mutex();
    
    ptrdiff_t delta = static_cast<std::byte*>(_trusted_memory) - 
                      static_cast<std::byte*>(other._trusted_memory);
    void* &root = accessRoot(_trusted_memory);
    if (root != nullptr)
    {
        root = static_cast<std::byte*>(root) + delta;
    }
    void* curr = ptrToBytes<void>(_trusted_memory, allocator_metadata_size);
    while (curr != nullptr)
    {
        void* &prev = accessPrevBlock(curr);
        if (prev != nullptr)
        {
            prev = static_cast<std::byte*>(prev) + delta;
        }
        
        void* &next = accessNextBlock(curr);
        if (next != nullptr)
        {
            next = static_cast<std::byte*>(next) + delta;
        }
        
        if (accessBlockData(curr).occupied)
        {
            void* &occParent = accessField<void*>(curr, occupiedParentOffset);
            if (occParent != nullptr)
            {
                occParent = static_cast<std::byte*>(occParent) + delta;
            }
        }
        else
        {
            void* &left = accessLeft(curr);
            if (left != nullptr)
            {
                left = static_cast<std::byte*>(left) + delta;
            }
            
            void* &right = accessRight(curr);
            if (right != nullptr)
            {
                right = static_cast<std::byte*>(right) + delta;
            }
            
            void* &freeParent = accessField<void*>(curr, freeParentOffset);
            if (freeParent != nullptr)
            {
                freeParent = static_cast<std::byte*>(freeParent) + delta;
            }
        }
        
        curr = accessNextBlock(curr);
    }
}

allocator_red_black_tree &allocator_red_black_tree::operator=(const allocator_red_black_tree &other)
{
    if (this != &other)
    {
        this->~allocator_red_black_tree();
        new (this) allocator_red_black_tree(other);
    }
    return *this;
}

bool allocator_red_black_tree::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

[[nodiscard]] void *allocator_red_black_tree::do_allocate_sm(
    size_t size)
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
    
    size_t minSize = size + occupied_block_metadata_size;
    // Возможно и такое, что занятый блок будет меньше свободного...
    if (minSize < free_block_metadata_size)
    {
	    minSize = free_block_metadata_size;
	}

    void* found = nullptr;
    allocator_with_fit_mode::fit_mode mode = accessFitMode(_trusted_memory);

    if (mode == allocator_with_fit_mode::fit_mode::the_best_fit || 
        mode == allocator_with_fit_mode::fit_mode::first_fit) 
    {
        void* curr = accessRoot(_trusted_memory);
        while (curr) {
            if (accessBlockFullSize(curr, _trusted_memory) >= minSize)
            {
                found = curr;
                curr = accessLeft(curr);
            } else {
                curr = accessRight(curr);
            }
        }
    } 
    else if (mode == allocator_with_fit_mode::fit_mode::the_worst_fit) 
    {
        void* curr = accessRoot(_trusted_memory);
        while (curr) {
            if (accessBlockFullSize(curr, _trusted_memory) >= minSize)
            {
                found = curr;
            }
            curr = accessRight(curr);
        }
    }

    if (!found)
    {
	    throw std::bad_alloc();
	}

    size_t fullFoundSize = accessBlockFullSize(found, _trusted_memory);
    removeFromTree(_trusted_memory, found);

    if (fullFoundSize >= minSize + free_block_metadata_size) {
        void* remainings = ptrToBytes<void>(found, minSize);
        
        void* oldNext = accessNextBlock(found);
        accessNextBlock(remainings) = oldNext;
        accessPrevBlock(remainings) = found;
        accessNextBlock(found) = remainings;
        
        if (oldNext) {
            accessPrevBlock(oldNext) = remainings;
        }

        insertIntoTree(_trusted_memory, remainings);
    }

    accessBlockData(found).occupied = true;
    return ptrToBytes<void>(found, occupied_block_metadata_size);
}


void allocator_red_black_tree::do_deallocate_sm(
    void *at)
{
    if (!at)
    {
	    return;
	}
	
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));

    void* block = ptrToBytes<void>(at, -static_cast<ptrdiff_t>(occupied_block_metadata_size));
    
    void* n = accessNextBlock(block);
    if (n && !accessBlockData(n).occupied)
    {
        removeFromTree(_trusted_memory, n);
        accessNextBlock(block) = accessNextBlock(n);
        if (accessNextBlock(n)) accessPrevBlock(accessNextBlock(n)) = block;
    }
    
    void* p = accessPrevBlock(block);
    if (p && !accessBlockData(p).occupied)
    {
        removeFromTree(_trusted_memory, p);
        accessNextBlock(p) = accessNextBlock(block);
        if (accessNextBlock(block)) accessPrevBlock(accessNextBlock(block)) = p;
        block = p;
    }

    insertIntoTree(_trusted_memory, block);
}

void allocator_red_black_tree::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
    accessFitMode(_trusted_memory) = mode;
}


std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info() const
{
    std::lock_guard<std::mutex> lock(accessMutex(_trusted_memory));
    std::vector<allocator_test_utils::block_info> result;
    void* curr = ptrToBytes<void>(_trusted_memory, allocator_metadata_size);
    while (curr) {
        result.push_back({ accessBlockFullSize(curr, _trusted_memory), accessBlockData(curr).occupied });
        curr = accessNextBlock(curr);
    }
    return result;
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info_inner() const
{
    return get_blocks_info();
}


allocator_red_black_tree::rb_iterator allocator_red_black_tree::begin() const noexcept
{
    return rb_iterator(_trusted_memory);
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::end() const noexcept
{
    return rb_iterator();
}

bool allocator_red_black_tree::rb_iterator::operator==(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return _block_ptr == other._block_ptr;
}

bool allocator_red_black_tree::rb_iterator::operator!=(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_red_black_tree::rb_iterator &allocator_red_black_tree::rb_iterator::operator++() & noexcept
{
    if (_block_ptr) {
        _block_ptr = accessNextBlock(_block_ptr);
    }
    return *this;
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::rb_iterator::operator++(int n)
{
    rb_iterator temp = *this;
    ++(*this);
    return temp;
}

size_t allocator_red_black_tree::rb_iterator::size() const noexcept
{
    return accessBlockFullSize(_block_ptr, _trusted);
}

void *allocator_red_black_tree::rb_iterator::operator*() const noexcept
{
    return _block_ptr;
}

allocator_red_black_tree::rb_iterator::rb_iterator() : _block_ptr(nullptr), _trusted(nullptr) {}

allocator_red_black_tree::rb_iterator::rb_iterator(void *trusted) : _block_ptr(ptrToBytes<void>(trusted, allocator_metadata_size)), _trusted(trusted) {}

bool allocator_red_black_tree::rb_iterator::occupied() const noexcept
{
    return accessBlockData(_block_ptr).occupied;
}

#pragma endregion