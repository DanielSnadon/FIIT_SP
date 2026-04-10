#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

#pragma region given

#ifndef SYS_PROG_B_PLUS_TREE_H
#define SYS_PROG_B_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BP_tree final : private compare //EBCO
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bptree_node_base
    {
        bool _is_terminate;

        bptree_node_base() noexcept;
        virtual ~bptree_node_base() =default;
    };

    struct bptree_node_term : public bptree_node_base
    {
        bptree_node_term* _next;

        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bptree_node_term() noexcept;
    };

    struct bptree_node_middle : public bptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

    // HelperFuncs

    bool keysEqual(const tkey& lhs, const tkey& rhs) const;
    bool isTerminateNode(const bptree_node_base* node) const noexcept;

    bptree_node_term* asTermNode(bptree_node_base* node) const noexcept;
    const bptree_node_term* asTermNode(const bptree_node_base* node) const noexcept;

    bptree_node_middle* asMiddleNode(bptree_node_base* node) const noexcept;
    const bptree_node_middle* asMiddleNode(const bptree_node_base* node) const noexcept;

    bptree_node_term* makeTermNode() const;
    bptree_node_middle* makeMiddleNode() const;

    void destroySubtree(bptree_node_base* node) noexcept;
    bptree_node_base* cloneSubtree(const bptree_node_base* node, bptree_node_term*& previousLeaf) const;

    bptree_node_term* leftmostLeaf(bptree_node_base* node) const noexcept;
    const bptree_node_term* leftmostLeaf(const bptree_node_base* node) const noexcept;

    size_t lowerIndexInTermNode(const bptree_node_term* node, const tkey& key) const;
    size_t upperIndexInTermNode(const bptree_node_term* node, const tkey& key) const;
    size_t childIndexInMiddleNode(const bptree_node_middle* node, const tkey& key) const;

    const tkey& firstKeyInSubtree(const bptree_node_base* node) const;
    void rebuildSeparators(bptree_node_base* node);

    struct split_result
    {
        bool happened;
        tkey separator;
        bptree_node_base* rightNode;
    };

    std::pair<bptree_node_term*, size_t> insertIntoSubtree(bptree_node_base* node, const tree_data_type& data, bool& inserted, split_result& splitResult);

    std::vector<tree_data_type> collectAllData() const;

public:

    // region constructors declaration

    explicit BP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BP_tree(const BP_tree& other);

    BP_tree(BP_tree&& other) noexcept;

    BP_tree& operator=(const BP_tree& other);

    BP_tree& operator=(BP_tree&& other) noexcept;

    ~BP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bptree_iterator;
    class bptree_const_iterator;

    class bptree_iterator final
    {
        bptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_iterator;

        friend class BP_tree;
        friend class bptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_iterator(bptree_node_term* node = nullptr, size_t index = 0);

    };

    class bptree_const_iterator final
    {
        const bptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_const_iterator;

        friend class BP_tree;
        friend class bptree_iterator;

        bptree_const_iterator(const bptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_const_iterator(const bptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;

    // endregion iterators declaration

    // region element access declaration

    /*
     * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
     */
    tvalue& at(const tkey&);
    const tvalue& at(const tkey&) const;

    /*
     * If key not exists, makes default initialization of value
     */
    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    // endregion element access declaration
    // region iterator begins declaration

    bptree_iterator begin();
    bptree_iterator end();

    bptree_const_iterator begin() const;
    bptree_const_iterator end() const;

    bptree_const_iterator cbegin() const;
    bptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bptree_iterator find(const tkey& key);
    bptree_const_iterator find(const tkey& key) const;

    bptree_iterator lower_bound(const tkey& key);
    bptree_const_iterator lower_bound(const tkey& key) const;

    bptree_iterator upper_bound(const tkey& key);
    bptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bptree_iterator insert_or_assign(const tree_data_type& data);
    bptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bptree_iterator erase(bptree_iterator pos);
    bptree_iterator erase(bptree_const_iterator pos);

    bptree_iterator erase(bptree_iterator beg, bptree_iterator en);
    bptree_iterator erase(bptree_const_iterator beg, bptree_const_iterator en);


    bptree_iterator erase(const tkey& key);

    // endregion modifiers declaration
};

#pragma endregion
#pragma region taskItself
// Непосредственно задача

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_pairs(const BP_tree::tree_data_type &lhs,
                                                     const BP_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_base::bptree_node_base() noexcept
{
    _is_terminate = false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_term::bptree_node_term() noexcept
{
    this->_is_terminate = true;
    _next = nullptr;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_middle::bptree_node_middle() noexcept
{
    this->_is_terminate = false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::value_type> BP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

#pragma region helperFuncs
// Вспомогательные функции

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::keysEqual(const tkey& lhs, const tkey& rhs) const
{
    return !compare_keys(lhs, rhs) && !compare_keys(rhs, lhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::isTerminateNode(const bptree_node_base* node) const noexcept
{
    return node != nullptr && node->_is_terminate;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term* BP_tree<tkey, tvalue, compare, t>::asTermNode(bptree_node_base* node) const noexcept
{
    return static_cast<bptree_node_term*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term* BP_tree<tkey, tvalue, compare, t>::asTermNode(const bptree_node_base* node) const noexcept
{
    return static_cast<const bptree_node_term*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_middle* BP_tree<tkey, tvalue, compare, t>::asMiddleNode(bptree_node_base* node) const noexcept
{
    return static_cast<bptree_node_middle*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const typename BP_tree<tkey, tvalue, compare, t>::bptree_node_middle* BP_tree<tkey, tvalue, compare, t>::asMiddleNode(const bptree_node_base* node) const noexcept
{
    return static_cast<const bptree_node_middle*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term* BP_tree<tkey, tvalue, compare, t>::makeTermNode() const
{
    return new bptree_node_term();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_middle* BP_tree<tkey, tvalue, compare, t>::makeMiddleNode() const
{
    return new bptree_node_middle();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::destroySubtree(bptree_node_base* node) noexcept
{
    if (node == nullptr)
    {
        return;
    }

    if (isTerminateNode(node))
    {
        delete asTermNode(node);
        return;
    }

    bptree_node_middle* middleNode = asMiddleNode(node);
    for (bptree_node_base* child : middleNode->_pointers)
    {
        destroySubtree(child);
    }

    delete middleNode;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_base* BP_tree<tkey, tvalue, compare, t>::cloneSubtree(
        const bptree_node_base* node,
        bptree_node_term*& previousLeaf) const
{
    if (node == nullptr)
    {
        return nullptr;
    }

    // Случай 1 - копируется лист
    if (isTerminateNode(node))
    {
        const bptree_node_term* sourceLeaf = asTermNode(node);
        bptree_node_term* clonedLeaf = makeTermNode();
        clonedLeaf->_data = sourceLeaf->_data;

        if (previousLeaf != nullptr)
        {
            previousLeaf->_next = clonedLeaf;
        }

        previousLeaf = clonedLeaf;
        return clonedLeaf;
    }

    // Случай 2 - копируем внутренний узел
    const bptree_node_middle* sourceMiddle = asMiddleNode(node);
    bptree_node_middle* clonedMiddle = makeMiddleNode();

    try
    {
        clonedMiddle->_keys = sourceMiddle->_keys;
        for (const bptree_node_base* child : sourceMiddle->_pointers)
        {
            clonedMiddle->_pointers.push_back(cloneSubtree(child, previousLeaf));
        }
    }
    catch (...)
    {
        destroySubtree(clonedMiddle);
        throw;
    }

    return clonedMiddle;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term* BP_tree<tkey, tvalue, compare, t>::leftmostLeaf(bptree_node_base* node) const noexcept
{
    if (node == nullptr)
    {
        return nullptr;
    }

    while (!isTerminateNode(node))
    {
        node = asMiddleNode(node)->_pointers.front();
    }

    return asTermNode(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term* BP_tree<tkey, tvalue, compare, t>::leftmostLeaf(const bptree_node_base* node) const noexcept
{
    if (node == nullptr)
    {
        return nullptr;
    }

    while (!isTerminateNode(node))
    {
        node = asMiddleNode(node)->_pointers.front();
    }

    return asTermNode(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::lowerIndexInTermNode(const bptree_node_term* node, const tkey& key) const
{
    size_t index = 0;
    while (index < node->_data.size() && compare_keys(node->_data[index].first, key))
    {
        ++index;
    }
    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::upperIndexInTermNode(const bptree_node_term* node, const tkey& key) const
{
    size_t index = 0;
    while (index < node->_data.size() && !compare_keys(key, node->_data[index].first))
    {
        ++index;
    }
    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::childIndexInMiddleNode(const bptree_node_middle* node, const tkey& key) const
{
    size_t childIndex = 0;
    while (childIndex < node->_keys.size() && !compare_keys(key, node->_keys[childIndex]))
    {
        ++childIndex;
    }
    return childIndex;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tkey& BP_tree<tkey, tvalue, compare, t>::firstKeyInSubtree(const bptree_node_base* node) const
{
    if (isTerminateNode(node))
    {
        return asTermNode(node)->_data.front().first;
    }

    return firstKeyInSubtree(asMiddleNode(node)->_pointers.front());
}

// Пересборка разделителей
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::rebuildSeparators(bptree_node_base* node)
{
    if (node == nullptr || isTerminateNode(node))
    {
        return;
    }

    bptree_node_middle* middleNode = asMiddleNode(node);

    for (bptree_node_base* child : middleNode->_pointers)
    {
        rebuildSeparators(child);
    }

    // Каждый ребёнок даёт разделитель (минимальный ключ)
    middleNode->_keys.clear();
    for (size_t index = 1; index < middleNode->_pointers.size(); ++index)
    {
        middleNode->_keys.push_back(firstKeyInSubtree(middleNode->_pointers[index]));
    }
}

// Рекурсивное добавление узла с перебалансировкой
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term*, size_t> BP_tree<tkey, tvalue, compare, t>::insertIntoSubtree(bptree_node_base* node, const tree_data_type& data, bool& inserted, split_result& splitResult)
{
    // Case A - мы в листе
    if (isTerminateNode(node))
    {
        bptree_node_term* termNode = asTermNode(node);
        size_t insertIndex = lowerIndexInTermNode(termNode, data.first);

        if (insertIndex < termNode->_data.size() && keysEqual(termNode->_data[insertIndex].first, data.first))
        { // Case A1 - такой ключ уже есть
            inserted = false;
            splitResult = { false, tkey(), nullptr };
            return { termNode, insertIndex };
        }

        termNode->_data.insert(termNode->_data.begin() + static_cast<std::ptrdiff_t>(insertIndex), data);
        inserted = true;

        if (termNode->_data.size() <= maximum_keys_in_node)
        { // Case A2 - переполнения нет
            splitResult = { false, tkey(), nullptr };
            return { termNode, insertIndex };
        }

        // Case A3 - переполнение есть
        bptree_node_term* rightNode = makeTermNode();
        const size_t oldSize = termNode->_data.size();
        const size_t splitIndex = t;
        

        for (size_t index = splitIndex; index < oldSize; ++index)
        {
            rightNode->_data.push_back(std::move(termNode->_data[index]));
        }

        termNode->_data.erase(termNode->_data.begin() + static_cast<std::ptrdiff_t>(splitIndex), termNode->_data.end());

        rightNode->_next = termNode->_next;
        termNode->_next = rightNode;

        splitResult = { true, rightNode->_data.front().first, rightNode };

        if (insertIndex < termNode->_data.size())
        {
            return { termNode, insertIndex };
        }

        return { rightNode, insertIndex - termNode->_data.size() };
    }

    // Case B - текущий узел является внутренним
    bptree_node_middle* middleNode = asMiddleNode(node);
    const size_t childIndex = childIndexInMiddleNode(middleNode, data.first);

    split_result childSplit = { false, tkey(), nullptr };
    std::pair<bptree_node_term*, size_t> iteratorState = insertIntoSubtree(middleNode->_pointers[childIndex], data, inserted, childSplit);

    // Case B1 - вставка не удалась или дочерний узел в пределах нормального размера
    if (!inserted)
    {
        splitResult = { false, tkey(), nullptr };
        return iteratorState;
    }

    if (!childSplit.happened)
    {
        splitResult = { false, tkey(), nullptr };
        return iteratorState;
    }

    // Case B2 - разделение произошло
    middleNode->_keys.insert(middleNode->_keys.begin() + static_cast<std::ptrdiff_t>(childIndex), childSplit.separator);
    middleNode->_pointers.insert(middleNode->_pointers.begin() + static_cast<std::ptrdiff_t>(childIndex + 1), childSplit.rightNode);

    if (middleNode->_keys.size() <= maximum_keys_in_node)
    { // Case B2.1 - переполнения у родителя нет
        splitResult = { false, tkey(), nullptr };
        return iteratorState;
    }

    // Case B2.2 - родитель переполнился
    bptree_node_middle* rightNode = makeMiddleNode();
    const size_t promoteIndex = t;
    const tkey promotedKey = middleNode->_keys[promoteIndex];

    for (size_t index = promoteIndex + 1; index < middleNode->_keys.size(); ++index)
    {
        rightNode->_keys.push_back(std::move(middleNode->_keys[index]));
    }

    for (size_t index = promoteIndex + 1; index < middleNode->_pointers.size(); ++index)
    {
        rightNode->_pointers.push_back(middleNode->_pointers[index]);
    }

    middleNode->_keys.erase(middleNode->_keys.begin() + static_cast<std::ptrdiff_t>(promoteIndex), middleNode->_keys.end());
    middleNode->_pointers.erase(middleNode->_pointers.begin() + static_cast<std::ptrdiff_t>(promoteIndex + 1), middleNode->_pointers.end());

    splitResult = { true, promotedKey, rightNode };
    return iteratorState;
}

// Сбор всех ключ-значение в один вектор
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::vector<typename BP_tree<tkey, tvalue, compare, t>::tree_data_type> BP_tree<tkey, tvalue, compare, t>::collectAllData() const
{
    std::vector<tree_data_type> allData;
    allData.reserve(_size);

    const bptree_node_term* currentNode = leftmostLeaf(_root);
    while (currentNode != nullptr)
    {
        for (const tree_data_type& item : currentNode->_data)
        {
            allData.push_back(item);
        }
        currentNode = currentNode->_next;
    }

    return allData;
}

#pragma endregion

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_iterator::operator*() const noexcept
{
    return *reinterpret_cast<value_type*>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::pointer BP_tree<tkey, tvalue, compare, t>::bptree_iterator
::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self & BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++()
{
    if (_node == nullptr)
    {
        return *this;
    }
    if (_index + 1 < _node->_data.size())
    {
        ++_index;
        return *this;
    }

    _node = _node->_next;
    _index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator==(const self &other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::current_node_keys_count() const noexcept
{
    return _node == nullptr ? 0 : _node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_iterator::bptree_iterator(bptree_node_term *node, size_t index)
{
    _node = node;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_iterator &it) noexcept
{
    _node = it._node;
    _index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator*() const noexcept
{
    return *reinterpret_cast<const value_type*>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::pointer BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self & BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++()
{
    if (_node == nullptr)
    {
        return *this;
    }

    if (_index + 1 < _node->_data.size())
    {
        ++_index;
        return *this;
    }

    _node = _node->_next;
    _index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator==(const self &other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node == nullptr ? 0 : _node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_node_term *node, size_t index)
{
    _node = node;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey &key)
{
    auto it = find(key);
    if (it == end())
    {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }

    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey &key) const
{
    auto it = find(key);
    if (it == end())
    {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }

    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](const tkey &key)
{
    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](tkey &&key)
{
    return emplace(std::move(key), tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(
    const tree_data_type &data)
{
    return emplace(data.first, data.second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const compare& cmp, pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(pp_allocator<value_type> alloc, const compare& cmp)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BP_tree<tkey, tvalue, compare, t>::BP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;

    for (; begin != end; ++begin)
    {
        insert(*begin);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;

    for (const auto& item : data)
    {
        insert(item);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const BP_tree& other)
{
    static_cast<compare&>(*this) = static_cast<const compare&>(other);
    _allocator = other._allocator;
    _size = other._size;

    bptree_node_term* previousLeaf = nullptr;
    _root = cloneSubtree(other._root, previousLeaf);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(BP_tree&& other) noexcept
{
    static_cast<compare&>(*this) = static_cast<compare&&>(other);
    _allocator = other._allocator;
    _root = other._root;
    _size = other._size;

    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(const BP_tree& other)
{
    if (this == &other)
    {
        return *this;
    }

    BP_tree copy(other);
    clear();

    static_cast<compare&>(*this) = static_cast<const compare&>(copy);
    _allocator = copy._allocator;
    _root = copy._root;
    _size = copy._size;

    copy._root = nullptr;
    copy._size = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(BP_tree&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    clear();

    static_cast<compare&>(*this) = static_cast<compare&&>(other);
    _allocator = other._allocator;
    _root = other._root;
    _size = other._size;

    other._root = nullptr;
    other._size = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::~BP_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::begin()
{
    return _root == nullptr ? end() : bptree_iterator(leftmostLeaf(_root), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::end()
{
    return bptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::begin() const
{
    return _root == nullptr ? end() : bptree_const_iterator(leftmostLeaf(_root), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::end() const
{
    return bptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

// return end()!
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto it = lower_bound(key);
    return it != end() && keysEqual(it->first, key) ? it : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    auto it = lower_bound(key);
    return it != end() && keysEqual(it->first, key) ? it : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr)
    {
        return end();
    }

    bptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        bptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    bptree_node_term* termNode = asTermNode(currentNode);
    size_t index = lowerIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bptree_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bptree_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    if (_root == nullptr)
    {
        return end();
    }

    const bptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        const bptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    const bptree_node_term* termNode = asTermNode(currentNode);
    size_t index = lowerIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bptree_const_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bptree_const_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr)
    {
        return end();
    }

    bptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        bptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    bptree_node_term* termNode = asTermNode(currentNode);
    size_t index = upperIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bptree_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bptree_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    if (_root == nullptr)
    {
        return end();
    }

    const bptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        const bptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    const bptree_node_term* termNode = asTermNode(currentNode);
    size_t index = upperIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bptree_const_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bptree_const_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    destroySubtree(_root);
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data.first), std::move(data.second));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);

    // Случай 1 - дерево пустое
    if (_root == nullptr)
    {
        bptree_node_term* rootLeaf = makeTermNode();
        rootLeaf->_data.push_back(data);
        _root = rootLeaf;
        _size = 1;
        return { bptree_iterator(rootLeaf, 0), true };
    }
    
    bool inserted = false;
    split_result splitResult = { false, tkey(), nullptr };
    std::pair<bptree_node_term*, size_t> iteratorState = insertIntoSubtree(_root, data, inserted, splitResult);
    
    // Случай 2 - ключ уже был
    if (!inserted)
    {
        return { bptree_iterator(iteratorState.first, iteratorState.second), false };
    }

    // Случай 3 - произошло разделение
    
    if (splitResult.happened)
    {
        bptree_node_middle* newRoot = makeMiddleNode();
        newRoot->_pointers.push_back(_root);
        newRoot->_keys.push_back(splitResult.separator);
        newRoot->_pointers.push_back(splitResult.rightNode);
        _root = newRoot;
    }

    rebuildSeparators(_root);
    ++_size;
    return { bptree_iterator(iteratorState.first, iteratorState.second), true };
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto [it, inserted] = emplace(data.first, data.second);
    if (!inserted)
    {
        it->second = data.second;
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    auto it = find(data.first);
    if (it != end())
    {
        it->second = std::move(data.second);
        return it;
    }

    return emplace(std::move(data.first), std::move(data.second)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto it = find(data.first);
    if (it != end())
    {
        it->second = std::move(data.second);
        return it;
    }

    return emplace(std::move(data.first), std::move(data.second)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator pos)
{
    if (pos == end())
    {
        return end();
    }

    std::optional<tkey> nextKey;
    bptree_iterator nextPos = pos;
    ++nextPos;
    if (nextPos != end())
    {
        nextKey = nextPos->first;
    }

    const tkey removedKey = pos->first;
    std::vector<tree_data_type> allData = collectAllData();
    clear();

    for (tree_data_type& item : allData)
    {
        if (!keysEqual(item.first, removedKey))
        {
            emplace(std::move(item.first), std::move(item.second));
        }
    }

    return nextKey.has_value() ? lower_bound(*nextKey) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator pos)
{
    return erase(bptree_iterator(const_cast<bptree_node_term*>(pos._node), pos._index));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator beg, bptree_iterator en)
{
    if (beg == en)
    {
        return en;
    }

    const tkey firstRemovedKey = beg->first;
    std::optional<tkey> stopKey;
    if (en != end())
    {
        stopKey = en->first;
    }

    std::vector<tree_data_type> allData = collectAllData();
    clear();

    for (tree_data_type& item : allData)
    {
        const bool afterLeft = !compare_keys(item.first, firstRemovedKey);

        const bool beforeRight = !stopKey.has_value() || compare_keys(item.first, *stopKey);

        if (afterLeft && beforeRight)
        {
            continue;
        }

        emplace(std::move(item.first), std::move(item.second));
    }

    return stopKey.has_value() ? lower_bound(*stopKey) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator beg, bptree_const_iterator en)
{
    return erase(bptree_iterator(const_cast<bptree_node_term*>(beg._node), beg._index), bptree_iterator(const_cast<bptree_node_term*>(en._node), en._index));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    bptree_iterator it = find(key);
    return it == end() ? end() : erase(it);
}

#pragma endregion

#endif

// cmake --build build
// ./build/associative_container/indexing_tree/b_plus_tree/tests/sys_prog_assctv_cntnr_indxng_tr_b_pls_tr_tests

// Ожидаю провал тестов.

// По условию имеем:
// 1. minimum_keys_in_node = t - 1; <- у листьев элементов минимум t-1.
// 2. Для обхода B+ деревьев мы проходим по листьям.

// Заглянем в тесты(1):
// Тесты ждут разбиение вида (t=3):
// [1,2,3] | [4] | [15,27]
// Wait a minute, но ведь в листе минимум может быть только t-1=3-1=2 элемента!

// :(