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

#ifndef SYS_PROG_BS_PLUS_TREE_H
#define SYS_PROG_BS_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BSP_tree final : private compare //EBCO
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:
    
    // Another restrictions :)
    static constexpr const size_t minimum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_non_root_keys_in_node = 3 * t - 1;
    static constexpr const size_t maximum_keys_in_node = 4 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bsptree_node_base
    {
        bool _is_terminated;

        bsptree_node_base() noexcept;
        virtual ~bsptree_node_base() = default;
    };

    struct bsptree_node_term : public bsptree_node_base
    {
        bsptree_node_term* _next;
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bsptree_node_term() noexcept;
    };

    struct bsptree_node_middle : public bsptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bsptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bsptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bsptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

    // HelperFuncs
    
    // Уже знакомое:
    bool keysEqual(const tkey& lhs, const tkey& rhs) const;
    bool isTerminateNode(const bsptree_node_base* node) const noexcept;

    bsptree_node_term* asTermNode(bsptree_node_base* node) const noexcept;
    const bsptree_node_term* asTermNode(const bsptree_node_base* node) const noexcept;
    bsptree_node_middle* asMiddleNode(bsptree_node_base* node) const noexcept;
    const bsptree_node_middle* asMiddleNode(const bsptree_node_base* node) const noexcept;
    bsptree_node_term* makeTermNode() const;
    bsptree_node_middle* makeMiddleNode() const;
    void destroySubtree(bsptree_node_base* node) noexcept;
    bsptree_node_base* cloneSubtree(const bsptree_node_base* node, bsptree_node_term*& previousLeaf) const;
    bsptree_node_term* leftmostLeaf(bsptree_node_base* node) const noexcept;
    const bsptree_node_term* leftmostLeaf(const bsptree_node_base* node) const noexcept;
    size_t lowerIndexInTermNode(const bsptree_node_term* node, const tkey& key) const;
    size_t upperIndexInTermNode(const bsptree_node_term* node, const tkey& key) const;
    size_t childIndexInMiddleNode(const bsptree_node_middle* node, const tkey& key) const;
    const tkey& firstKeyInSubtree(const bsptree_node_base* node) const;
    
    void rebuildOneSeparator(bsptree_node_middle* node);

    // New:
    size_t maxKeysForNode(const bsptree_node_base* node) const noexcept;
    size_t minKeysForNode(const bsptree_node_base* node) const noexcept;
    bool nodeOverflowed(const bsptree_node_base* node) const noexcept;
    bool nodeUnderflowed(const bsptree_node_base* node) const noexcept;

    struct split_result
    {
        bool happened;
        tkey separator;
        bsptree_node_base* rightNode;
    };

    struct erase_result
    {
        bool removed;
        bool underflow;
    };

    std::pair<bsptree_node_term*, size_t> insertIntoSubtree(bsptree_node_base* node, const tree_data_type& data, bool& inserted, split_result& splitResult);
    
    void fixChildOverflow(bsptree_node_middle* parent, size_t childIndex);
    void giveToOtherLeafs(bsptree_node_middle* parent, size_t leftIndex);
    void twoThreeLeafs(bsptree_node_middle* parent, size_t leftIndex);
    void giveToOtherMiddle(bsptree_node_middle* parent, size_t leftIndex);
    void twoThreeMiddle(bsptree_node_middle* parent, size_t leftIndex);
    void splitRoot();

    erase_result eraseFromSubtree(bsptree_node_base*& node, const tkey& key);
    void fixChildUnderflow(bsptree_node_middle* parent, size_t childIndex);

    void borrowFromLeftLeaf(bsptree_node_middle* parent, size_t childIndex);
    void borrowFromRightLeaf(bsptree_node_middle* parent, size_t childIndex);
    void ThreeTwoLeafs(bsptree_node_middle* parent, size_t leftIndex);
    void TwoOneLeafs(bsptree_node_middle* parent, size_t leftIndex);

    void borrowFromLeftMiddle(bsptree_node_middle* parent, size_t childIndex);
    void borrowFromRightMiddle(bsptree_node_middle* parent, size_t childIndex);
    void ThreeTwoMiddle(bsptree_node_middle* parent, size_t leftIndex);
    void TwoOneMiddle(bsptree_node_middle* parent, size_t leftIndex);

public:

    // region constructors declaration
    
    explicit BSP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());
    
    explicit BSP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BSP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());
    
    // endregion constructors declaration

    // region five declaration
    
    BSP_tree(const BSP_tree& other);
    
    BSP_tree(BSP_tree&& other) noexcept;
    
    BSP_tree& operator=(const BSP_tree& other);
    
    BSP_tree& operator=(BSP_tree&& other) noexcept;
    
    ~BSP_tree() noexcept;
    
    // endregion five declaration

    // region iterators declaration
    
    class bsptree_iterator;
    class bsptree_const_iterator;

    class bsptree_iterator final
    {
        bsptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_iterator;

        friend class BSP_tree;
        friend class bsptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;
        
        self& operator++();
        self operator++(int);
        
        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;
        
        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;
        
        explicit bsptree_iterator(bsptree_node_term* node = nullptr, size_t index = 0);
    };

    class bsptree_const_iterator final
    {
        const bsptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_const_iterator;

        friend class BSP_tree;
        friend class bsptree_iterator;

        bsptree_const_iterator(const bsptree_iterator& it) noexcept;
        
        reference operator*() const noexcept;
        pointer operator->() const noexcept;
        
        self& operator++();
        self operator++(int);
        
        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;
        
        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;
        
        explicit bsptree_const_iterator(const bsptree_node_term* node = nullptr, size_t index = 0);
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

    bsptree_iterator begin();
    bsptree_iterator end();

    bsptree_const_iterator begin() const;
    bsptree_const_iterator end() const;

    bsptree_const_iterator cbegin() const;
    bsptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bsptree_iterator find(const tkey& key);
    bsptree_const_iterator find(const tkey& key) const;

    bsptree_iterator lower_bound(const tkey& key);
    bsptree_const_iterator lower_bound(const tkey& key) const;

    bsptree_iterator upper_bound(const tkey& key);
    bsptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bsptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bsptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bsptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bsptree_iterator insert_or_assign(const tree_data_type& data);
    bsptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bsptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bsptree_iterator erase(bsptree_iterator pos);
    bsptree_iterator erase(bsptree_const_iterator pos);

    bsptree_iterator erase(bsptree_iterator beg, bsptree_iterator en);
    bsptree_iterator erase(bsptree_const_iterator beg, bsptree_const_iterator en);


    bsptree_iterator erase(const tkey& key);
    
    // endregion modifiers declaration
};

template<std::input_iterator iterator,
         comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
         std::size_t t = 5,
         typename U>
BSP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<U> = pp_allocator<U>())
    -> BSP_tree<typename std::iterator_traits<iterator>::value_type::first_type,
                typename std::iterator_traits<iterator>::value_type::second_type,
                compare,
                t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<U> = pp_allocator<U>())
    -> BSP_tree<tkey, tvalue, compare, t>;
    
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_base::bsptree_node_base() noexcept
{
    _is_terminated = false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term::bsptree_node_term() noexcept
{
    this->_is_terminated = true;
    _next = nullptr;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle::bsptree_node_middle() noexcept
{
    this->_is_terminated = false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BSP_tree<tkey, tvalue, compare, t>::value_type> BSP_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
{
    return _allocator;
}

#pragma region helperFuncs
// Вспомогательные функции

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::keysEqual(const tkey& lhs, const tkey& rhs) const
{
    return !compare_keys(lhs, rhs) && !compare_keys(rhs, lhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::isTerminateNode(const bsptree_node_base* node) const noexcept
{
    return node != nullptr && node->_is_terminated;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term* BSP_tree<tkey, tvalue, compare, t>::asTermNode(bsptree_node_base* node) const noexcept
{
    return static_cast<bsptree_node_term*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term* BSP_tree<tkey, tvalue, compare, t>::asTermNode(const bsptree_node_base* node) const noexcept
{
    return static_cast<const bsptree_node_term*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle* BSP_tree<tkey, tvalue, compare, t>::asMiddleNode(bsptree_node_base* node) const noexcept
{
    return static_cast<bsptree_node_middle*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle* BSP_tree<tkey, tvalue, compare, t>::asMiddleNode(const bsptree_node_base* node) const noexcept
{
    return static_cast<const bsptree_node_middle*>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term* BSP_tree<tkey, tvalue, compare, t>::makeTermNode() const
{
    return new bsptree_node_term();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle* BSP_tree<tkey, tvalue, compare, t>::makeMiddleNode() const
{
    return new bsptree_node_middle();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::destroySubtree(bsptree_node_base* node) noexcept
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

    bsptree_node_middle* middleNode = asMiddleNode(node);
    for (bsptree_node_base* child : middleNode->_pointers)
    {
        destroySubtree(child);
    }
    delete middleNode;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_base* BSP_tree<tkey, tvalue, compare, t>::cloneSubtree(const bsptree_node_base* node, bsptree_node_term*& previousLeaf) const
{
    if (node == nullptr)
    {
        return nullptr;
    }
    if (isTerminateNode(node))
    {
        const bsptree_node_term* sourceLeaf = asTermNode(node);
        bsptree_node_term* clonedLeaf = makeTermNode();
        clonedLeaf->_data = sourceLeaf->_data;
        if (previousLeaf != nullptr)
        {
            previousLeaf->_next = clonedLeaf;
        }
        previousLeaf = clonedLeaf;
        return clonedLeaf;
    }

    const bsptree_node_middle* sourceMiddle = asMiddleNode(node);
    bsptree_node_middle* clonedMiddle = makeMiddleNode();
    try
    {
        clonedMiddle->_keys = sourceMiddle->_keys;
        for (const bsptree_node_base* child : sourceMiddle->_pointers)
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
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term* BSP_tree<tkey, tvalue, compare, t>::leftmostLeaf(bsptree_node_base* node) const noexcept
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
const typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term* BSP_tree<tkey, tvalue, compare, t>::leftmostLeaf(const bsptree_node_base* node) const noexcept
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
size_t BSP_tree<tkey, tvalue, compare, t>::lowerIndexInTermNode(const bsptree_node_term* node, const tkey& key) const
{
    size_t index = 0;
    while (index < node->_data.size() && compare_keys(node->_data[index].first, key))
    {
        ++index;
    }
    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::upperIndexInTermNode(const bsptree_node_term* node, const tkey& key) const
{
    size_t index = 0;
    while (index < node->_data.size() && !compare_keys(key, node->_data[index].first))
    {
        ++index;
    }
    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::childIndexInMiddleNode(const bsptree_node_middle* node, const tkey& key) const
{
    size_t childIndex = 0;
    while (childIndex < node->_keys.size() && !compare_keys(key, node->_keys[childIndex]))
    {
        ++childIndex;
    }
    return childIndex;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tkey& BSP_tree<tkey, tvalue, compare, t>::firstKeyInSubtree(const bsptree_node_base* node) const
{
    if (isTerminateNode(node))
    {
        return asTermNode(node)->_data.front().first;
    }
    return firstKeyInSubtree(asMiddleNode(node)->_pointers.front());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey& lhs, const tkey& rhs) const
{
    return compare::operator()(lhs, rhs);
}

// New segment

/// Перестройка сепаратора у одного узла
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::rebuildOneSeparator(bsptree_node_middle* node)
{
    if (node == nullptr)
    {
        return;
    }

    node->_keys.clear();
    for (size_t index = 1; index < node->_pointers.size(); ++index)
    {
        node->_keys.push_back(firstKeyInSubtree(node->_pointers[index]));
    }
}

/// Узнаём максимальное число ключей для ноды (узел/корень?)
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::maxKeysForNode(const bsptree_node_base* node) const noexcept
{
    return node == _root ? maximum_keys_in_node : maximum_non_root_keys_in_node;
}

/// Узнаём минимальное число ключей для ноды (узел/корень?)
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::minKeysForNode(const bsptree_node_base* node) const noexcept
{
    return node == _root ? 1 : minimum_keys_in_node;
}

/// Переполнение?
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::nodeOverflowed(const bsptree_node_base* node) const noexcept
{
    if (node == nullptr)
    {
        return false;
    }

    if (isTerminateNode(node))
    {
        return asTermNode(node)->_data.size() > maxKeysForNode(node);
    }
    return asMiddleNode(node)->_keys.size() > maxKeysForNode(node);
}

/// Нехватка?
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::nodeUnderflowed(const bsptree_node_base* node) const noexcept
{
    if (node == nullptr || node == _root)
    {
        return false;
    }
    if (isTerminateNode(node))
    {
        return asTermNode(node)->_data.size() < minimum_keys_in_node;
    }
    return asMiddleNode(node)->_keys.size() < minimum_keys_in_node;
}

/// {Листья} Поделиться с соседом
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::giveToOtherLeafs(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_term* leftLeaf = asTermNode(parent->_pointers[leftIndex]);
    bsptree_node_term* rightLeaf = asTermNode(parent->_pointers[leftIndex + 1]);

    std::vector<tree_data_type> combined;
    combined.reserve(leftLeaf->_data.size() + rightLeaf->_data.size());
    for (const tree_data_type& item : leftLeaf->_data) combined.push_back(item);
    for (const tree_data_type& item : rightLeaf->_data) combined.push_back(item);

    const size_t leftCount = combined.size() / 2;
    const size_t rightCount = combined.size() - leftCount;

    leftLeaf->_data.clear();
    rightLeaf->_data.clear();
    for (size_t i = 0; i < leftCount; ++i) leftLeaf->_data.push_back(std::move(combined[i]));
    for (size_t i = 0; i < rightCount; ++i) rightLeaf->_data.push_back(std::move(combined[leftCount + i]));

    rebuildOneSeparator(parent);
}

/// {Листья} Двое при переполнении превращаются в трое
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::twoThreeLeafs(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_term* leftLeaf = asTermNode(parent->_pointers[leftIndex]);
    bsptree_node_term* middleLeaf = asTermNode(parent->_pointers[leftIndex + 1]);
    bsptree_node_term* rightLeaf = makeTermNode();

    std::vector<tree_data_type> combined;
    combined.reserve(leftLeaf->_data.size() + middleLeaf->_data.size());
    for (const tree_data_type& item : leftLeaf->_data) combined.push_back(item);
    for (const tree_data_type& item : middleLeaf->_data) combined.push_back(item);

    const size_t firstCount = combined.size() / 3;
    const size_t secondCount = (combined.size() - firstCount) / 2;
    const size_t thirdCount = combined.size() - firstCount - secondCount;

    leftLeaf->_data.clear();
    middleLeaf->_data.clear();
    rightLeaf->_data.clear();
    for (size_t i = 0; i < firstCount; ++i) leftLeaf->_data.push_back(std::move(combined[i]));
    for (size_t i = 0; i < secondCount; ++i) middleLeaf->_data.push_back(std::move(combined[firstCount + i]));
    for (size_t i = 0; i < thirdCount; ++i) rightLeaf->_data.push_back(std::move(combined[firstCount + secondCount + i]));

    rightLeaf->_next = middleLeaf->_next;
    middleLeaf->_next = rightLeaf;

    parent->_pointers.insert(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(leftIndex + 2), rightLeaf);
    rebuildOneSeparator(parent);
}

/// {Узлы} Поделиться с соседом
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::giveToOtherMiddle(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_middle* leftMiddle = asMiddleNode(parent->_pointers[leftIndex]);
    bsptree_node_middle* rightMiddle = asMiddleNode(parent->_pointers[leftIndex + 1]);

    std::vector<bsptree_node_base*> combined;
    combined.reserve(leftMiddle->_pointers.size() + rightMiddle->_pointers.size());
    for (bsptree_node_base* child : leftMiddle->_pointers) combined.push_back(child);
    for (bsptree_node_base* child : rightMiddle->_pointers) combined.push_back(child);

    const size_t leftPointerCount = combined.size() / 2;
    const size_t rightPointerCount = combined.size() - leftPointerCount;

    leftMiddle->_pointers.clear();
    rightMiddle->_pointers.clear();
    for (size_t i = 0; i < leftPointerCount; ++i) leftMiddle->_pointers.push_back(combined[i]);
    for (size_t i = 0; i < rightPointerCount; ++i) rightMiddle->_pointers.push_back(combined[leftPointerCount + i]);

    rebuildOneSeparator(leftMiddle);
    rebuildOneSeparator(rightMiddle);
    rebuildOneSeparator(parent);
}

/// {Узлы} Двое при переполнении превращаются в трое
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::twoThreeMiddle(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_middle* leftMiddle = asMiddleNode(parent->_pointers[leftIndex]);
    bsptree_node_middle* middleMiddle = asMiddleNode(parent->_pointers[leftIndex + 1]);
    bsptree_node_middle* rightMiddle = makeMiddleNode();

    std::vector<bsptree_node_base*> combined;
    combined.reserve(leftMiddle->_pointers.size() + middleMiddle->_pointers.size());
    for (bsptree_node_base* child : leftMiddle->_pointers) combined.push_back(child);
    for (bsptree_node_base* child : middleMiddle->_pointers) combined.push_back(child);

    const size_t firstPointerCount = combined.size() / 3;
    const size_t secondPointerCount = (combined.size() - firstPointerCount) / 2;
    const size_t thirdPointerCount = combined.size() - firstPointerCount - secondPointerCount;

    leftMiddle->_pointers.clear();
    middleMiddle->_pointers.clear();
    rightMiddle->_pointers.clear();
    for (size_t i = 0; i < firstPointerCount; ++i) leftMiddle->_pointers.push_back(combined[i]);
    for (size_t i = 0; i < secondPointerCount; ++i) middleMiddle->_pointers.push_back(combined[firstPointerCount + i]);
    for (size_t i = 0; i < thirdPointerCount; ++i) rightMiddle->_pointers.push_back(combined[firstPointerCount + secondPointerCount + i]);

    parent->_pointers.insert(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(leftIndex + 2), rightMiddle);
    rebuildOneSeparator(leftMiddle);
    rebuildOneSeparator(middleMiddle);
    rebuildOneSeparator(rightMiddle);
    rebuildOneSeparator(parent);
}

/// Фикс переполнения
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::fixChildOverflow(bsptree_node_middle* parent, size_t childIndex)
{
    if (!nodeOverflowed(parent->_pointers[childIndex]))
    {
        return;
    }
    
    // Случай 1 - заём слева
    if (childIndex > 0)
    {
        bsptree_node_base* leftSibling = parent->_pointers[childIndex - 1];
        bsptree_node_base* child = parent->_pointers[childIndex];
        if (isTerminateNode(leftSibling) == isTerminateNode(child))
        {
            bool canUseLeft = isTerminateNode(leftSibling)
                ? asTermNode(leftSibling)->_data.size() < maximum_non_root_keys_in_node
                : asMiddleNode(leftSibling)->_keys.size() < maximum_non_root_keys_in_node;
            if (canUseLeft)
            {
                if (isTerminateNode(child)) giveToOtherLeafs(parent, childIndex - 1);
                else giveToOtherMiddle(parent, childIndex - 1);
                return;
            }
        }
    }
    
    // Случай 2 - заём справа
    if (childIndex + 1 < parent->_pointers.size())
    {
        bsptree_node_base* rightSibling = parent->_pointers[childIndex + 1];
        bsptree_node_base* child = parent->_pointers[childIndex];
        if (isTerminateNode(rightSibling) == isTerminateNode(child))
        {
            bool canUseRight = isTerminateNode(rightSibling)
                ? asTermNode(rightSibling)->_data.size() < maximum_non_root_keys_in_node
                : asMiddleNode(rightSibling)->_keys.size() < maximum_non_root_keys_in_node;
            if (canUseRight)
            {
                if (isTerminateNode(child)) giveToOtherLeafs(parent, childIndex);
                else giveToOtherMiddle(parent, childIndex);
                return;
            }
        }
    }
    
    // Случай 3 - split
    if (childIndex > 0)
    {
        if (isTerminateNode(parent->_pointers[childIndex])) twoThreeLeafs(parent, childIndex - 1);
        else twoThreeMiddle(parent, childIndex - 1);
    }
    else
    {
        if (isTerminateNode(parent->_pointers[childIndex])) twoThreeLeafs(parent, childIndex);
        else twoThreeMiddle(parent, childIndex);
    }
}

/// Рекурсивный алгоритм вставки
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term*, size_t>
BSP_tree<tkey, tvalue, compare, t>::insertIntoSubtree(bsptree_node_base* node, const tree_data_type& data, bool& inserted, split_result& splitResult)
{
    if (isTerminateNode(node))
    {
        bsptree_node_term* termNode = asTermNode(node);
        size_t insertIndex = lowerIndexInTermNode(termNode, data.first);
        if (insertIndex < termNode->_data.size() && keysEqual(termNode->_data[insertIndex].first, data.first))
        {
            inserted = false;
            splitResult = { false, tkey(), nullptr };
            return { termNode, insertIndex };
        }

        termNode->_data.insert(termNode->_data.begin() + static_cast<std::ptrdiff_t>(insertIndex), data);
        inserted = true;
        splitResult = { false, tkey(), nullptr };
        return { termNode, insertIndex };
    }

    bsptree_node_middle* middleNode = asMiddleNode(node);
    size_t childIndex = childIndexInMiddleNode(middleNode, data.first);

    split_result childSplit = { false, tkey(), nullptr };
    auto iteratorState = insertIntoSubtree(middleNode->_pointers[childIndex], data, inserted, childSplit);

    if (!inserted)
    {
        splitResult = { false, tkey(), nullptr };
        return iteratorState;
    }

    if (nodeOverflowed(middleNode->_pointers[childIndex]))
    {
        fixChildOverflow(middleNode, childIndex);
        rebuildOneSeparator(middleNode);
    }

    splitResult = { false, tkey(), nullptr };
    return iteratorState;
}

/// Переполнение у корня решается отдельно
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::splitRoot()
{
    if (_root == nullptr || !nodeOverflowed(_root))
    {
        return;
    }
    
    // Случай 1 - корень=лист
    if (isTerminateNode(_root))
    {
        bsptree_node_term* oldRoot = asTermNode(_root);
        bsptree_node_term* rightLeaf = makeTermNode();
        bsptree_node_middle* newRoot = makeMiddleNode();

        const size_t splitIndex = oldRoot->_data.size() / 2;
        const size_t oldSize = oldRoot->_data.size();
        for (size_t i = splitIndex; i < oldSize; ++i)
        {
            rightLeaf->_data.push_back(std::move(oldRoot->_data[i]));
        }
        oldRoot->_data.erase(oldRoot->_data.begin() + static_cast<std::ptrdiff_t>(splitIndex), oldRoot->_data.end());

        rightLeaf->_next = oldRoot->_next;
        oldRoot->_next = rightLeaf;

        newRoot->_pointers.push_back(oldRoot);
        newRoot->_pointers.push_back(rightLeaf);
        rebuildOneSeparator(newRoot);
        _root = newRoot;
        return;
    }
    
    // Случай 2 - корень=узел
    bsptree_node_middle* oldRoot = asMiddleNode(_root);
    bsptree_node_middle* rightNode = makeMiddleNode();
    bsptree_node_middle* newRoot = makeMiddleNode();

    const size_t leftPointerCount = oldRoot->_pointers.size() / 2;
    while (oldRoot->_pointers.size() > leftPointerCount)
    {
        rightNode->_pointers.insert(rightNode->_pointers.begin(), oldRoot->_pointers.back());
        oldRoot->_pointers.pop_back();
    }

    rebuildOneSeparator(oldRoot);
    rebuildOneSeparator(rightNode);
    newRoot->_pointers.push_back(oldRoot);
    newRoot->_pointers.push_back(rightNode);
    rebuildOneSeparator(newRoot);
    _root = newRoot;
}

/// {Листья} взять у соседа слева
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::borrowFromLeftLeaf(bsptree_node_middle* parent, size_t childIndex)
{
    bsptree_node_term* leftLeaf = asTermNode(parent->_pointers[childIndex - 1]);
    bsptree_node_term* childLeaf = asTermNode(parent->_pointers[childIndex]);
    childLeaf->_data.insert(childLeaf->_data.begin(), std::move(leftLeaf->_data.back()));
    leftLeaf->_data.pop_back();
    rebuildOneSeparator(parent);
}

/// {Листья} взять у соседа справа
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::borrowFromRightLeaf(bsptree_node_middle* parent, size_t childIndex)
{
    bsptree_node_term* childLeaf = asTermNode(parent->_pointers[childIndex]);
    bsptree_node_term* rightLeaf = asTermNode(parent->_pointers[childIndex + 1]);
    childLeaf->_data.push_back(std::move(rightLeaf->_data.front()));
    rightLeaf->_data.erase(rightLeaf->_data.begin());
    rebuildOneSeparator(parent);
}

/// {Листья} слияние трёх узлов с листьями в два
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::ThreeTwoLeafs(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_term* first = asTermNode(parent->_pointers[leftIndex]);
    bsptree_node_term* second = asTermNode(parent->_pointers[leftIndex + 1]);
    bsptree_node_term* third = asTermNode(parent->_pointers[leftIndex + 2]);

    std::vector<tree_data_type> combined;
    combined.reserve(first->_data.size() + second->_data.size() + third->_data.size());
    for (const tree_data_type& item : first->_data) combined.push_back(item);
    for (const tree_data_type& item : second->_data) combined.push_back(item);
    for (const tree_data_type& item : third->_data) combined.push_back(item);

    const size_t firstCount = combined.size() / 2;
    const size_t secondCount = combined.size() - firstCount;

    first->_data.clear();
    second->_data.clear();
    for (size_t i = 0; i < firstCount; ++i) first->_data.push_back(std::move(combined[i]));
    for (size_t i = 0; i < secondCount; ++i) second->_data.push_back(std::move(combined[firstCount + i]));

    second->_next = third->_next;
    delete third;
    parent->_pointers.erase(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(leftIndex + 2));
    rebuildOneSeparator(parent);
}

/// {Листья} слияние двух узлов в один (special для корня)
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::TwoOneLeafs(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_term* leftLeaf = asTermNode(parent->_pointers[leftIndex]);
    bsptree_node_term* rightLeaf = asTermNode(parent->_pointers[leftIndex + 1]);
    for (tree_data_type& item : rightLeaf->_data) leftLeaf->_data.push_back(std::move(item));
    leftLeaf->_next = rightLeaf->_next;
    delete rightLeaf;
    parent->_pointers.erase(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(leftIndex + 1));
    rebuildOneSeparator(parent);
}

/// {Узлы} взять у соседа слева
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::borrowFromLeftMiddle(bsptree_node_middle* parent, size_t childIndex)
{
    bsptree_node_middle* leftMiddle = asMiddleNode(parent->_pointers[childIndex - 1]);
    bsptree_node_middle* childMiddle = asMiddleNode(parent->_pointers[childIndex]);
    childMiddle->_pointers.insert(childMiddle->_pointers.begin(), leftMiddle->_pointers.back());
    leftMiddle->_pointers.pop_back();
    rebuildOneSeparator(leftMiddle);
    rebuildOneSeparator(childMiddle);
    rebuildOneSeparator(parent);
}

/// {Узлы} взять у соседа справа
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::borrowFromRightMiddle(bsptree_node_middle* parent, size_t childIndex)
{
    bsptree_node_middle* childMiddle = asMiddleNode(parent->_pointers[childIndex]);
    bsptree_node_middle* rightMiddle = asMiddleNode(parent->_pointers[childIndex + 1]);
    childMiddle->_pointers.push_back(rightMiddle->_pointers.front());
    rightMiddle->_pointers.erase(rightMiddle->_pointers.begin());
    rebuildOneSeparator(childMiddle);
    rebuildOneSeparator(rightMiddle);
    rebuildOneSeparator(parent);
}

/// {Узлы} слияние трёх узлов с листьями в два
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::ThreeTwoMiddle(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_middle* first = asMiddleNode(parent->_pointers[leftIndex]);
    bsptree_node_middle* second = asMiddleNode(parent->_pointers[leftIndex + 1]);
    bsptree_node_middle* third = asMiddleNode(parent->_pointers[leftIndex + 2]);

    std::vector<bsptree_node_base*> combined;
    combined.reserve(first->_pointers.size() + second->_pointers.size() + third->_pointers.size());
    for (bsptree_node_base* child : first->_pointers) combined.push_back(child);
    for (bsptree_node_base* child : second->_pointers) combined.push_back(child);
    for (bsptree_node_base* child : third->_pointers) combined.push_back(child);

    const size_t firstCount = combined.size() / 2;
    const size_t secondCount = combined.size() - firstCount;

    first->_pointers.clear();
    second->_pointers.clear();
    for (size_t i = 0; i < firstCount; ++i) first->_pointers.push_back(combined[i]);
    for (size_t i = 0; i < secondCount; ++i) second->_pointers.push_back(combined[firstCount + i]);

    delete third;
    parent->_pointers.erase(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(leftIndex + 2));
    rebuildOneSeparator(first);
    rebuildOneSeparator(second);
    rebuildOneSeparator(parent);
}

/// {Узлы} слияние двух узлов в один (special для корня)
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::TwoOneMiddle(bsptree_node_middle* parent, size_t leftIndex)
{
    bsptree_node_middle* leftMiddle = asMiddleNode(parent->_pointers[leftIndex]);
    bsptree_node_middle* rightMiddle = asMiddleNode(parent->_pointers[leftIndex + 1]);
    for (bsptree_node_base* child : rightMiddle->_pointers) leftMiddle->_pointers.push_back(child);
    delete rightMiddle;
    parent->_pointers.erase(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(leftIndex + 1));
    rebuildOneSeparator(leftMiddle);
    rebuildOneSeparator(parent);
}

/// Фикс недостатка
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::fixChildUnderflow(bsptree_node_middle* parent, size_t childIndex)
{
    bsptree_node_base* child = parent->_pointers[childIndex];
    if (!nodeUnderflowed(child))
    {
        return;
    }
    
    // Случай Ь - ребёнок-лист
    if (isTerminateNode(child))
    {
	    // Случай Ь1 - попытка занять у левого соседа
        if (childIndex > 0)
        {
            bsptree_node_term* leftLeaf = asTermNode(parent->_pointers[childIndex - 1]);
            if (leftLeaf->_data.size() > minimum_keys_in_node)
            {
                borrowFromLeftLeaf(parent, childIndex);
                return;
            }
        }
        // Случай Ь2 - попытка занять у правого соседа
        if (childIndex + 1 < parent->_pointers.size())
        {
            bsptree_node_term* rightLeaf = asTermNode(parent->_pointers[childIndex + 1]);
            if (rightLeaf->_data.size() > minimum_keys_in_node)
            {
                borrowFromRightLeaf(parent, childIndex);
                return;
            }
        }
        // Случай Ь3 - корень с двумя детьми
        if (parent == _root && parent->_pointers.size() == 2)
        {
            TwoOneLeafs(parent, 0);
            return;
        }
        
        // Случай Ь4 - слияние с двумя какими-нибудь соседями
        if (childIndex > 0 && childIndex + 1 < parent->_pointers.size())
        {
            ThreeTwoLeafs(parent, childIndex - 1);
            return;
        }
        if (childIndex + 2 < parent->_pointers.size())
        {
            ThreeTwoLeafs(parent, childIndex);
            return;
        }
        if (childIndex >= 2)
        {
            ThreeTwoLeafs(parent, childIndex - 2);
            return;
        }
        
        // Случай Ь5 - крайний случай (придётся слить с одним соседом)
        if (childIndex > 0) TwoOneLeafs(parent, childIndex - 1);
        else if (childIndex + 1 < parent->_pointers.size()) TwoOneLeafs(parent, childIndex);
        return;
    }
    
    // Случай Ъ - ребёнок-узел
    
    // Случай Ъ1 - попытка занять у левого соседа
    if (childIndex > 0)
    {
        bsptree_node_middle* leftMiddle = asMiddleNode(parent->_pointers[childIndex - 1]);
        if (leftMiddle->_keys.size() > minimum_keys_in_node)
        {
            borrowFromLeftMiddle(parent, childIndex);
            return;
        }
    }
    // Случай Ъ2 - попытка занять у правого соседа
    if (childIndex + 1 < parent->_pointers.size())
    {
        bsptree_node_middle* rightMiddle = asMiddleNode(parent->_pointers[childIndex + 1]);
        if (rightMiddle->_keys.size() > minimum_keys_in_node)
        {
            borrowFromRightMiddle(parent, childIndex);
            return;
        }
    }
    // Случай Ъ3 - корень с двумя детьми
    if (parent == _root && parent->_pointers.size() == 2)
    {
        TwoOneMiddle(parent, 0);
        return;
    }
    
    // Случай Ъ4 - слияние с двумя какими-нибудь соседями
    if (childIndex > 0 && childIndex + 1 < parent->_pointers.size())
    {
        ThreeTwoMiddle(parent, childIndex - 1);
        return;
    }
    if (childIndex + 2 < parent->_pointers.size())
    {
        ThreeTwoMiddle(parent, childIndex);
        return;
    }
    if (childIndex >= 2)
    {
        ThreeTwoMiddle(parent, childIndex - 2);
        return;
    }
    
    // Случай Ъ5 - крайний случай (придётся слить с одним соседом)
    if (childIndex > 0) TwoOneMiddle(parent, childIndex - 1);
    else if (childIndex + 1 < parent->_pointers.size()) TwoOneMiddle(parent, childIndex);
}

/// Рекурсивный алгоритм удаления
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::erase_result BSP_tree<tkey, tvalue, compare, t>::eraseFromSubtree(bsptree_node_base*& node, const tkey& key)
{
    if (node == nullptr)
    {
        return { false, false };
    }

    if (isTerminateNode(node))
    {
        bsptree_node_term* termNode = asTermNode(node);
        size_t eraseIndex = lowerIndexInTermNode(termNode, key);
        if (eraseIndex >= termNode->_data.size() || !keysEqual(termNode->_data[eraseIndex].first, key))
        {
            return { false, false };
        }
        termNode->_data.erase(termNode->_data.begin() + static_cast<std::ptrdiff_t>(eraseIndex));
        return { true, nodeUnderflowed(termNode) };
    }

    bsptree_node_middle* middleNode = asMiddleNode(node);
    size_t childIndex = childIndexInMiddleNode(middleNode, key);
    erase_result childResult = eraseFromSubtree(middleNode->_pointers[childIndex], key);
    if (!childResult.removed)
    {
        return { false, false };
    }
    if (childResult.underflow)
    {
        fixChildUnderflow(middleNode, childIndex);
    }
    rebuildOneSeparator(middleNode);
    return { true, nodeUnderflowed(middleNode) };
}

#pragma endregion

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_node_term* node, size_t index) : _node(node), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const compare& cmp, pp_allocator<value_type> alloc) : _allocator(alloc), _root(nullptr), _size(0)
{
    static_cast<compare&>(*this) = cmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(pp_allocator<value_type> alloc, const compare& cmp) : _allocator(alloc), _root(nullptr), _size(0)
{
    static_cast<compare&>(*this) = cmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc) : _allocator(alloc), _root(nullptr), _size(0)
{
    static_cast<compare&>(*this) = cmp;
    for (; begin != end; ++begin) 
    {
        insert(*begin);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc) : _allocator(alloc), _root(nullptr), _size(0)
{
    static_cast<compare&>(*this) = cmp;
    for (const auto& item : data) insert(item);
}

// endregion BSP_tree constructor implementations

// region BSP_tree copy and move constructors

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const BSP_tree& other)
{
    static_cast<compare&>(*this) = static_cast<const compare&>(other);
    _allocator = other._allocator;
    _size = other._size;
    bsptree_node_term* previousLeaf = nullptr;
    _root = cloneSubtree(other._root, previousLeaf);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(BSP_tree&& other) noexcept : _allocator(other._allocator), _root(other._root), _size(other._size)
{
    static_cast<compare&>(*this) = static_cast<compare&&>(other);
    other._root = nullptr;
    other._size = 0;
}

// endregion BSP_tree copy and move constructors

// region BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(const BSP_tree& other)
{
    if (this == &other) return *this;
    BSP_tree copy(other);
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
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(BSP_tree&& other) noexcept
{
    if (this == &other) return *this;
    clear();
    static_cast<compare&>(*this) = static_cast<compare&&>(other);
    _allocator = other._allocator;
    _root = other._root;
    _size = other._size;
    other._root = nullptr;
    other._size = 0;
    return *this;
}

// endregion BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::~BSP_tree() noexcept
{
    clear();
}

// region BSP_tree iterators implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::bsptree_iterator(bsptree_node_term* node, size_t index) : _node(node), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator*() const noexcept
{
    return *reinterpret_cast<value_type*>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::self& BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++()
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
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::self BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::current_node_keys_count() const noexcept
{
    return _node == nullptr ? 0 : _node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_iterator& it) noexcept : _node(it._node), _index(it._index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator*() const noexcept
{
    return *reinterpret_cast<const value_type*>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++()
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
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node == nullptr ? 0 : _node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::index() const noexcept
{
    return _index;
}

// endregion BSP_tree iterators implementations

// region BSP_tree element access implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end())
    {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }

    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end())
    {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }

    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    return emplace(std::move(key), tvalue()).first->second;
}

// endregion BSP_tree element access implementations

// region BSP_tree iterator begins implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::begin()
{
    return _root == nullptr ? end() : bsptree_iterator(leftmostLeaf(_root), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::end()
{
    return bsptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::begin() const
{
    return _root == nullptr ? end() : bsptree_const_iterator(leftmostLeaf(_root), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::end() const
{
    return bsptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

// endregion BSP_tree iterator begins implementations

// region BSP_tree lookup implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto it = lower_bound(key);
    return it != end() && keysEqual(it->first, key) ? it : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    auto it = lower_bound(key);
    return it != end() && keysEqual(it->first, key) ? it : end();
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr)
    {
        return end();
    }

    bsptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        bsptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    bsptree_node_term* termNode = asTermNode(currentNode);
    size_t index = lowerIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bsptree_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bsptree_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    if (_root == nullptr)
    {
        return end();
    }

    const bsptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        const bsptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    const bsptree_node_term* termNode = asTermNode(currentNode);
    size_t index = lowerIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bsptree_const_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bsptree_const_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr)
    {
        return end();
    }

    bsptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        bsptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    bsptree_node_term* termNode = asTermNode(currentNode);
    size_t index = upperIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bsptree_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bsptree_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    if (_root == nullptr)
    {
        return end();
    }

    const bsptree_node_base* currentNode = _root;
    while (!isTerminateNode(currentNode))
    {
        const bsptree_node_middle* middleNode = asMiddleNode(currentNode);
        currentNode = middleNode->_pointers[childIndexInMiddleNode(middleNode, key)];
    }

    const bsptree_node_term* termNode = asTermNode(currentNode);
    size_t index = upperIndexInTermNode(termNode, key);

    if (index < termNode->_data.size())
    {
        return bsptree_const_iterator(termNode, index);
    }

    return termNode->_next == nullptr ? end() : bsptree_const_iterator(termNode->_next, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion BSP_tree lookup implementations

// region BSP_tree modifiers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    destroySubtree(_root);
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data.first, data.second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data.first), std::move(data.second));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);

    if (_root == nullptr)
    {
        bsptree_node_term* rootLeaf = makeTermNode();
        rootLeaf->_data.push_back(data);
        _root = rootLeaf;
        _size = 1;
        return { bsptree_iterator(rootLeaf, 0), true };
    }

    bool inserted = false;
    split_result splitResult = { false, tkey(), nullptr };
    auto iteratorState = insertIntoSubtree(_root, data, inserted, splitResult);

    if (!inserted)
    {
        return { bsptree_iterator(iteratorState.first, iteratorState.second), false };
    }

    splitRoot();
    if (!isTerminateNode(_root)) rebuildOneSeparator(asMiddleNode(_root));
    ++_size;
    return { bsptree_iterator(iteratorState.first, iteratorState.second), true };
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto result = emplace(data.first, data.second);
    auto it = result.first;
    bool inserted = result.second;

    if (!inserted) 
    {
        it->second = data.second;
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
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
template<typename ...Args>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
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
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator pos)
{
    if (pos == end())
    {
        return end();
    }

    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator pos)
{
    return erase(bsptree_iterator(const_cast<bsptree_node_term*>(pos._node), pos._index));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator beg, bsptree_iterator en)
{
    if (beg == en)
    {
        return en;
    }

    std::optional<tkey> stopKey;
    if (en != end())
    {
        stopKey = en->first;
    }

    if (!stopKey.has_value())
    {
        while (beg != end())
        {
            beg = erase(beg);
        }

        return end();
    }

    while (beg != end() && compare_keys(beg->first, *stopKey))
    {
        beg = erase(beg);
    }

    return lower_bound(*stopKey);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator beg, bsptree_const_iterator en)
{
    return erase(bsptree_iterator(const_cast<bsptree_node_term*>(beg._node), beg._index), bsptree_iterator(const_cast<bsptree_node_term*>(en._node), en._index));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);
    if (it == end()) return end();

    std::optional<tkey> nextKey;
    auto nextIt = it;
    ++nextIt;
    if (nextIt != end()) nextKey = nextIt->first;

    erase_result result = eraseFromSubtree(_root, key);
    if (!result.removed) return end();
    --_size;

    if (_root != nullptr && isTerminateNode(_root))
    {
        bsptree_node_term* rootLeaf = asTermNode(_root);
        if (rootLeaf->_data.empty())
        {
            delete rootLeaf;
            _root = nullptr;
        }
    }
    else if (_root != nullptr)
    {
        bsptree_node_middle* rootMiddle = asMiddleNode(_root);
        rebuildOneSeparator(rootMiddle);
        if (rootMiddle->_pointers.size() == 1)
        {
            bsptree_node_base* newRoot = rootMiddle->_pointers.front();
            rootMiddle->_pointers.clear();
            delete rootMiddle;
            _root = newRoot;
        }
        else if (rootMiddle->_pointers.empty())
        {
            delete rootMiddle;
            _root = nullptr;
        }
    }

    return nextKey.has_value() ? lower_bound(*nextKey) : end();
}

// endregion BSP_tree modifiers implementations

#endif

// cmake --build build
// ./build/associative_container/indexing_tree/b_star_plus_tree/tests/sys_prog_assctv_cntnr_indxng_tr_b_str_pls_tr_tests 