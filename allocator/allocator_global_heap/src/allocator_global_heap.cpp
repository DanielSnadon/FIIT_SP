#include <not_implemented.h>
#include "../include/allocator_global_heap.h"

// Тесты запускаются из корня:
// ./build/allocator/allocator_global_heap/tests/sys_prog_allctr_allctr_glbl_hp_tests

allocator_global_heap::allocator_global_heap()
{
}

[[nodiscard]] void *allocator_global_heap::do_allocate_sm(
    size_t size)
{
    if (size == 0) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex);
    
    return ::operator new(size);
}

void allocator_global_heap::do_deallocate_sm(
    void *at)
{
    if (at == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);

    ::operator delete(at);
}

allocator_global_heap::~allocator_global_heap()
{
}

allocator_global_heap::allocator_global_heap(const allocator_global_heap &other)
{
}

allocator_global_heap &allocator_global_heap::operator=(const allocator_global_heap &other)
{
    return *this;
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return dynamic_cast<const allocator_global_heap*>(&other);
}

allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept
{
}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept
{
    return *this;
}

// Тесты запускаются из корня:
// ./build/allocator/allocator_global_heap/tests/sys_prog_allctr_allctr_glbl_hp_tests