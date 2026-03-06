#include <iostream>
#include <string>
#include "include/allocator_global_heap.h"

struct cords {
    int x;
    int y;

    cords(int x_val, int y_val) : x(x_val), y(y_val) {}
};

int main() {
    try {
        allocator_global_heap alloc;

        int* a = new (alloc.allocate(sizeof(int))) int(555);
        double* b = new (alloc.allocate(sizeof(double))) double(3.1415);
        std::string* c = new (alloc.allocate(sizeof(std::string))) std::string("Toaster");
        cords* d = new (alloc.allocate(sizeof(cords))) cords(10, 20);

        std::cout << "*a = " << *a << '\n';
        std::cout << "*b = " << *b << '\n';
        std::cout << "*c = " << *c << '\n';
        std::cout << "d = (" << d->x << ", " << d->y << ")\n";

        alloc.deallocate(a, sizeof(int), alignof(int));
        alloc.deallocate(b, sizeof(double), alignof(double));
        alloc.deallocate(c, sizeof(std::string), alignof(std::string));
        alloc.deallocate(d, sizeof(cords), alignof(cords));
    }
    catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}

// Запускать из корня:
// ./build/allocator/allocator_global_heap/demonstration