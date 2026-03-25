#include <iostream>
#include <memory>
#include "allocator_red_black_tree.h"

static void print_blocks(const allocator_red_black_tree& alloc, const char* title)
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
    allocator_red_black_tree alloc(
        3000,
        nullptr,
        allocator_with_fit_mode::fit_mode::first_fit
    );

    print_blocks(alloc, "Начальное состояние");

    void* a = alloc.allocate(250);
    void* b = alloc.allocate(250);
    print_blocks(alloc, "После двух аллокейтов");

    alloc.deallocate(a, 1);
    print_blocks(alloc, "После deallocate(a)");

    alloc.set_fit_mode(allocator_with_fit_mode::fit_mode::the_best_fit);
    void* c = alloc.allocate(229);
    print_blocks(alloc, "После allocate(229) с best_fit");

    alloc.deallocate(b, 1);
    alloc.deallocate(c, 1);
    print_blocks(alloc, "После полной очистки");

    return 0;
}

// TEST IT
// cmake --build build
// ./build/allocator/allocator_red_black_tree/red_black_demo 