#include <iostream>
#include "../include/allocator_boundary_tags.h"

int main()
{
    allocator_boundary_tags alloc(
        2048,
        nullptr,
        allocator_with_fit_mode::fit_mode::first_fit
    );

    int* int_value = static_cast<int*>(alloc.allocate(sizeof(int), alignof(int)));
    *int_value = 555;

    double* double_value = static_cast<double*>(alloc.allocate(sizeof(double), alignof(double)));
    *double_value = 3.14159;

    char* text = static_cast<char*>(alloc.allocate(sizeof(char) * 16, alignof(char)));
    text[0] = 'H';
    text[1] = 'i';
    text[2] = '!';
    text[3] = '\0';

    std::cout << "int: " << *int_value << '\n';
    std::cout << "double: " << *double_value << '\n';
    std::cout << "texto: " << text << '\n';

    alloc.deallocate(int_value, sizeof(int), alignof(int));
    alloc.deallocate(double_value, sizeof(double), alignof(double));
    alloc.deallocate(text, sizeof(char) * 16, alignof(char));

    return 0;
}

// cmake -S . -B build
// cmake --build build
// Запускать из корня:
// ./build/allocator/allocator_boundary_tags/demonstration3