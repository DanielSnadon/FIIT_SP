#include <iostream>
#include <memory>
#include "./include/allocator_buddies_system.h"

static void print_blocks(const allocator_buddies_system& alloc, const char* title)
{
    std::cout << "\n" << title << "\n";
    auto blocks = alloc.get_blocks_info();
    for (size_t i = 0; i < blocks.size(); ++i)
    {
        std::cout
            << "block[" << i << "]: size = " << blocks[i].block_size
            << ", occupied = " << (blocks[i].is_block_occupied ? "true" : "false")
            << '\n';
    }
}

int main()
{
    allocator_buddies_system alloc(
        256,
        nullptr,
        allocator_with_fit_mode::fit_mode::first_fit
    );

    print_blocks(alloc, "Начальное состояние");

    void* a = alloc.allocate(40);
    print_blocks(alloc, "После allocate(40)");

    void* b = alloc.allocate(20);
    print_blocks(alloc, "После allocate(20)");

    alloc.deallocate(a, 1);
    print_blocks(alloc, "После deallocate(a)");

    alloc.deallocate(b, 1);
    print_blocks(alloc, "После deallocate(b)");

    return 0;
}

// TEST IT
// cmake --build build
// ./build/allocator/allocator_buddies_system/buddy_system_demo 