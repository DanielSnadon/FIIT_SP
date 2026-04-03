#include "../include/b_tree.h"

#include <iostream>
#include <string>

void printSeparator()
{
    std::cout << "----------------------------------------\n";
}

template<typename Tree>
void printTreeInOrder(const Tree& tree, const std::string& title)
{
    std::cout << title << "\n";

    if (tree.empty())
    {
        std::cout << "  <Дерево пустое>\n";
        return;
    }

    for (auto iterator = tree.cbegin(); iterator != tree.cend(); ++iterator)
    {
        // depth() показывает глубину, на которой лежит текущий ключ.
        // index() показывает позицию ключа внутри узла.
        std::cout
            << "  ключ = " << iterator->first
            << ", значение = " << iterator->second
            << ", глубина = " << iterator.depth()
            << ", индекс_в_узле = " << iterator.index()
            << "\n";
    }
}

int main()
{
    using Tree = B_tree<int, std::string, std::less<int>, 5>;

    Tree tree(std::less<int>(), nullptr);

    printSeparator();

    tree.emplace(1,   "a");
    tree.emplace(2,   "b");
    tree.emplace(15,  "c");
    tree.emplace(3,   "d");
    tree.emplace(4,   "e");
    tree.emplace(100, "f");
    tree.emplace(24,  "g");
    tree.emplace(456, "h");
    tree.emplace(101, "j");
    tree.emplace(45,  "k");
    tree.emplace(193, "l");
    tree.emplace(534, "m");

    std::cout << "Tree size after insertions: " << tree.size() << "\n\n";
    printTreeInOrder(tree, "All elements in sorted order:");

    printSeparator();
    std::cout << "SEARCH DEMO\n";
    printSeparator();

    std::cout << "contains(24)  = " << (tree.contains(24) ? "true" : "false") << "\n";
    std::cout << "contains(999) = " << (tree.contains(999) ? "true" : "false") << "\n";

    auto foundIterator = tree.find(100);
    if (foundIterator != tree.end())
    {
        std::cout << "find(100): ключ = " << foundIterator->first
                  << ", значение = " << foundIterator->second << "\n";
    }

    std::cout << "at(45) = " << tree.at(45) << "\n";

    printSeparator();

    std::cout << "erase(24)\n";
    tree.erase(24);

    std::cout << "erase(1)\n";
    tree.erase(1);

    std::cout << "erase(534)\n";
    tree.erase(534);

    std::cout << "Tree size after erasures: " << tree.size() << "\n\n";
    printTreeInOrder(tree, "Elements after erase operations:");

    printSeparator();

    auto lowerBoundIterator = tree.lower_bound(44);
    if (lowerBoundIterator != tree.end())
    {
        std::cout << "lower_bound(44) -> "
                  << lowerBoundIterator->first << ":" << lowerBoundIterator->second << "\n";
    }

    auto upperBoundIterator = tree.upper_bound(45);
    if (upperBoundIterator != tree.end())
    {
        std::cout << "upper_bound(45) -> "
                  << upperBoundIterator->first << ":" << upperBoundIterator->second << "\n";
    }
    else
    {
        std::cout << "upper_bound(45) -> end()\n";
    }

    printSeparator();

    return 0;
}

// TEST IT
// cmake --build build
// ./build/associative_container/indexing_tree/b_tree/b_tree_demo