#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <stdexcept>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>
#include <optional>

#ifndef SYS_PROG_BS_TREE_H
#define SYS_PROG_BS_TREE_H

#pragma region given
// Дано по условию

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BS_tree final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    // TODO: Another restrictions
    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bstree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bstree_node*, maximum_keys_in_node + 2> _pointers;
        bstree_node() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bstree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;


private:

    static bool is_leaf_node(const bstree_node* node) noexcept;
    bstree_node* create_node() const;
    void destroy_node(bstree_node* node) const noexcept;
    void destroy_subtree(bstree_node* node) noexcept;
    bstree_node* clone_subtree(const bstree_node* node) const;
    size_t lower_index_in_node(const bstree_node* node, const tkey& key) const;
    size_t upper_index_in_node(const bstree_node* node, const tkey& key) const;
    std::stack<std::pair<bstree_node**, size_t>> make_leftmost_path();
    std::stack<std::pair<bstree_node**, size_t>> make_rightmost_path();
    void merge_children(bstree_node* parent, size_t key_index);
    void borrow_from_left(bstree_node* parent, size_t child_index);
    void borrow_from_right(bstree_node* parent, size_t child_index);
    tree_data_type max_key_in_subtree(const bstree_node* node) const;
    tree_data_type min_key_in_subtree(const bstree_node* node) const;
    bool erase_from_subtree(bstree_node** node_link, const tkey& key);

    void createNodeFromList(
            bstree_node* node,
            const std::vector<tree_data_type>& keys,
            size_t keyBegin,
            size_t keyCount,
            const std::vector<bstree_node*>& pointers,
            size_t pointerBegin);

    void collectTwoChildren(
            const bstree_node* left,
            const tree_data_type& separator,
            const bstree_node* right,
            std::vector<tree_data_type>& keys,
            std::vector<bstree_node*>& pointers);

    bool insertIntoSubtree(bstree_node* node, const tree_data_type& data);
    void redistributeTwoChildren(bstree_node* parent, size_t leftIndex);
    void twoThree(bstree_node* parent, size_t leftIndex);
    void fixChildOverflow(bstree_node* parent, size_t childIndex);
    void splitRoot();

public:

    // region constructors declaration

    explicit BS_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BS_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BS_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BS_tree(const BS_tree& other);

    BS_tree(BS_tree&& other) noexcept;

    BS_tree& operator=(const BS_tree& other);

    BS_tree& operator=(BS_tree&& other) noexcept;

    ~BS_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bstree_iterator;
    class bstree_reverse_iterator;
    class bstree_const_iterator;
    class bstree_const_reverse_iterator;

    class bstree_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_iterator;

        friend class B_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);

    };

    class bstree_const_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_iterator;

        friend class B_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_const_iterator(const bstree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_const_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    class bstree_reverse_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_reverse_iterator;

        friend class B_tree;
        friend class bstree_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_reverse_iterator(const bstree_iterator& it) noexcept;
        operator bstree_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_reverse_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);
    };

    class bstree_const_reverse_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_reverse_iterator;

        friend class B_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_iterator;

        bstree_const_reverse_iterator(const bstree_reverse_iterator& it) noexcept;
        operator bstree_const_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_const_reverse_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class bstree_iterator;
    friend class bstree_const_iterator;
    friend class bstree_reverse_iterator;
    friend class bstree_const_reverse_iterator;

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

    bstree_iterator begin();
    bstree_iterator end();

    bstree_const_iterator begin() const;
    bstree_const_iterator end() const;

    bstree_const_iterator cbegin() const;
    bstree_const_iterator cend() const;

    bstree_reverse_iterator rbegin();
    bstree_reverse_iterator rend();

    bstree_const_reverse_iterator rbegin() const;
    bstree_const_reverse_iterator rend() const;

    bstree_const_reverse_iterator crbegin() const;
    bstree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bstree_iterator find(const tkey& key);
    bstree_const_iterator find(const tkey& key) const;

    bstree_iterator lower_bound(const tkey& key);
    bstree_const_iterator lower_bound(const tkey& key) const;

    bstree_iterator upper_bound(const tkey& key);
    bstree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bstree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bstree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bstree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bstree_iterator insert_or_assign(const tree_data_type& data);
    bstree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bstree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bstree_iterator erase(bstree_iterator pos);
    bstree_iterator erase(bstree_const_iterator pos);

    bstree_iterator erase(bstree_iterator beg, bstree_iterator en);
    bstree_iterator erase(bstree_const_iterator beg, bstree_const_iterator en);


    bstree_iterator erase(const tkey& key);

    // endregion modifiers declaration
};

#pragma endregion
#pragma region helperFuncs
// Вспомогательные функции

template<typename T>
std::vector<T> stackToVector(std::stack<T> stack)
{
	std::vector<T> result;
	while (!stack.empty()) {
		result.push_back(stack.top());
		stack.pop();
	}
	std::reverse(result.begin(), result.end());
	return result;
}

template<typename T>
std::stack<T> vectorToStack(const std::vector<T>& frames)
{
	std::stack<T> result;
	for (const auto& frame : frames) {
		result.push(frame);
	}
	return result;
}

template<typename T>
bool equalStacks(std::stack<T> left, std::stack<T> right)
{
	if (left.size() != right.size()) {
		return false;
	}

	auto leftVector = stackToVector(left);
	auto rightVector = stackToVector(right);

	for (std::size_t i = 0; i < leftVector.size(); ++i) {
		if (leftVector[i].first != rightVector[i].first || leftVector[i].second != rightVector[i].second) {
			return false;
		}
	}

	return true;
}

#pragma endregion
#pragma region taskItself
// Непосредственно задача

template<typename Key, typename Compare>
bool keysEqual(const Key& left, const Key& right, const Compare& comp)
{
    return !comp(left, right) && !comp(right, left);
}

template<std::input_iterator iterator,
         comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
         std::size_t t = 5,
         typename U>
BS_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<U> = pp_allocator<U>())
    -> BS_tree<typename std::iterator_traits<iterator>::value_type::first_type,
               typename std::iterator_traits<iterator>::value_type::second_type,
               compare,
               t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<U> = pp_allocator<U>())
    -> BS_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_node::bstree_node() noexcept {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BS_tree<tkey, tvalue, compare, t>::value_type> BS_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::is_leaf_node(const bstree_node* node) noexcept
{
    return node == nullptr || node->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_node* BS_tree<tkey, tvalue, compare, t>::create_node() const
{
    return new bstree_node();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::destroy_node(bstree_node* node) const noexcept
{
    delete node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::destroy_subtree(bstree_node* node) noexcept
{
    if (node == nullptr) {
        return;
    }

    for (bstree_node* child : node->_pointers) {
        destroy_subtree(child);
    }

    destroy_node(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_node* BS_tree<tkey, tvalue, compare, t>::clone_subtree(const bstree_node* node) const
{
    if (node == nullptr) {
        return nullptr;
    }

    bstree_node* copy = create_node();
    try
    {
        copy->_keys = node->_keys;
        for (bstree_node* child : node->_pointers) {
            copy->_pointers.push_back(clone_subtree(child));
        }

    } catch (...) {
        destroy_subtree(copy);
        throw;
    }

    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::lower_index_in_node(const bstree_node* node, const tkey& key) const
{
    size_t index = 0;
    while (index < node->_keys.size() && compare_keys(node->_keys[index].first, key)) {
        ++index;
    }

    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::upper_index_in_node(const bstree_node* node, const tkey& key) const
{
    size_t index = 0;
    while (index < node->_keys.size() && !compare_keys(key, node->_keys[index].first)) {
        ++index;
    }

    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::stack<std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_node**, size_t>>
BS_tree<tkey, tvalue, compare, t>::make_leftmost_path()
{
    std::stack<std::pair<bstree_node**, size_t>> path;
    
    if (_root == nullptr) {
        return path;
    }

    bstree_node** currentLink = &_root;
    size_t childIndex = 0;

    while (*currentLink != nullptr) {
        path.push({currentLink, childIndex});

        if (is_leaf_node(*currentLink)) {
            break;
        }

        currentLink = &((*currentLink)->_pointers[0]);
        childIndex = 0;
    }

    return path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::stack<std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_node**, size_t>>
BS_tree<tkey, tvalue, compare, t>::make_rightmost_path()
{
    std::stack<std::pair<bstree_node**, size_t>> path;
    if (_root == nullptr) {
        return path;
    }

    bstree_node** currentLink = &_root;
    size_t childIndex = 0;

    while (*currentLink != nullptr) {
        path.push({currentLink, childIndex});
        if (is_leaf_node(*currentLink)) {
            break;
        }
        childIndex = (*currentLink)->_pointers.size() - 1;
        currentLink = &((*currentLink)->_pointers[childIndex]);
    }

    return path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::merge_children(bstree_node* parent, size_t key_index)
{
    bstree_node* leftChild = parent->_pointers[key_index];
    bstree_node* rightChild = parent->_pointers[key_index + 1];

    leftChild->_keys.push_back(std::move(parent->_keys[key_index]));
    for (auto& keyValue : rightChild->_keys) {
        leftChild->_keys.push_back(std::move(keyValue));
    }
    for (bstree_node* child : rightChild->_pointers) {
        leftChild->_pointers.push_back(child);
    }

    parent->_keys.erase(parent->_keys.begin() + static_cast<std::ptrdiff_t>(key_index));
    parent->_pointers.erase(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(key_index + 1));

    destroy_node(rightChild);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::borrow_from_left(bstree_node* parent, size_t child_index)
{
    bstree_node* leftSibling = parent->_pointers[child_index - 1];
    bstree_node* child = parent->_pointers[child_index];

    child->_keys.insert(child->_keys.begin(), std::move(parent->_keys[child_index - 1]));
    parent->_keys[child_index - 1] = std::move(leftSibling->_keys.back());
    leftSibling->_keys.pop_back();

    if (!is_leaf_node(leftSibling)) {
        child->_pointers.insert(child->_pointers.begin(), leftSibling->_pointers.back());
        leftSibling->_pointers.pop_back();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::borrow_from_right(bstree_node* parent, size_t child_index)
{
    bstree_node* child = parent->_pointers[child_index];
    bstree_node* rightSibling = parent->_pointers[child_index + 1];

    child->_keys.push_back(std::move(parent->_keys[child_index]));
    parent->_keys[child_index] = std::move(rightSibling->_keys.front());
    rightSibling->_keys.erase(rightSibling->_keys.begin());

    if (!is_leaf_node(rightSibling)) {
        child->_pointers.push_back(rightSibling->_pointers.front());
        rightSibling->_pointers.erase(rightSibling->_pointers.begin());
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::tree_data_type
BS_tree<tkey, tvalue, compare, t>::max_key_in_subtree(const bstree_node* node) const
{
    while (!is_leaf_node(node)) {
        node = node->_pointers.back();
    }
    return node->_keys.back();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::tree_data_type
BS_tree<tkey, tvalue, compare, t>::min_key_in_subtree(const bstree_node* node) const
{
    while (!is_leaf_node(node)) {
        node = node->_pointers.front();
    }
    return node->_keys.front();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::erase_from_subtree(bstree_node** node_link, const tkey& key)
{
    bstree_node* node = *node_link;
    if (node == nullptr) {
        return false;
    }

    const compare& cmp = static_cast<const compare&>(*this);
    size_t keyIndex = lower_index_in_node(node, key);
    bool foundInNode = keyIndex < node->_keys.size() && keysEqual(node->_keys[keyIndex].first, key, cmp);

    if (foundInNode) { // Случаи типа А: ключ найден в текущем узле
        if (is_leaf_node(node)) {
            // Случай A1 - удаляемый узел - лист
            node->_keys.erase(node->_keys.begin() + static_cast<std::ptrdiff_t>(keyIndex));
        } else if (node->_pointers[keyIndex]->_keys.size() >= t) {
            // Случай A2 - узел не лист, но у брата слева есть замена
            tree_data_type replacer = max_key_in_subtree(node->_pointers[keyIndex]);
            node->_keys[keyIndex] = replacer;
            erase_from_subtree(&node->_pointers[keyIndex], replacer.first);
        } else if (node->_pointers[keyIndex + 1]->_keys.size() >= t) {
            // Случай A3 - узел не лист, но у брата справа есть замена
            tree_data_type replacer = min_key_in_subtree(node->_pointers[keyIndex + 1]);
            node->_keys[keyIndex] = replacer;
            erase_from_subtree(&node->_pointers[keyIndex + 1], replacer.first);
        } else {
            // Случай A4 - оба брата не имеют замены
            merge_children(node, keyIndex);
            erase_from_subtree(&node->_pointers[keyIndex], key);
        }

    } else { // Случаи типа B: ключ НЕ найден в текущем узле
        if (is_leaf_node(node)) {
            // Случай B1 - мы уже в листе, но ключ всё ещё не найден
            return false;
        }

        size_t childIndex = keyIndex;
        bstree_node* child = node->_pointers[childIndex];
        // Случай B2 - рекурсивный спуск
        if (child->_keys.size() == minimum_keys_in_node) {
            if (childIndex > 0 && node->_pointers[childIndex - 1]->_keys.size() >= t) {
                borrow_from_left(node, childIndex);
            } else if (childIndex + 1 < node->_pointers.size() && node->_pointers[childIndex + 1]->_keys.size() >= t) {
                borrow_from_right(node, childIndex);
            } else if (childIndex + 1 < node->_pointers.size()) {
                merge_children(node, childIndex);
            } else {
                merge_children(node, childIndex - 1);
                --childIndex;
            }
        }

        erase_from_subtree(&node->_pointers[childIndex], key);
    }

    // Случаи типа C: обработка корня
    node = *node_link;
    if (node == _root && node != nullptr && node->_keys.empty()) {
        if (node->_pointers.empty()) {
            // Случай C1 - полное опустошение дерева
            destroy_node(node);
            _root = nullptr;
            *node_link = nullptr;
        } else {
            // Случай C2 - наследие корня
            bstree_node* newRoot = node->_pointers.front();
            node->_pointers.clear();
            destroy_node(node);
            _root = newRoot;
            *node_link = newRoot;
        }
    }

    return true;
}

// Собрать узел из массивов ключей и детей
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::createNodeFromList(
    bstree_node* node,
    const std::vector<tree_data_type>& keys,
    size_t key_begin,
    size_t key_count,
    const std::vector<bstree_node*>& pointers,
    size_t pointer_begin)
{
    node->_keys.clear();
    node->_pointers.clear();

    for (size_t i = 0; i < key_count; ++i) {
        node->_keys.push_back(keys[key_begin + i]);
    }
    if (!pointers.empty()) {
        for (size_t i = 0; i < key_count + 1; ++i) {
            node->_pointers.push_back(pointers[pointer_begin + i]);
        }
    }
}

// Склеить два соседних ребёнка и разделяющий ключ родителя в один буфер для дальнейшего перераспределения
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::collectTwoChildren(
    const bstree_node* left,
    const tree_data_type& separator,
    const bstree_node* right,
    std::vector<tree_data_type>& keys,
    std::vector<bstree_node*>& pointers)
{
    keys.clear();
    pointers.clear();

    keys.reserve(left->_keys.size() + 1 + right->_keys.size());
    for (const auto& key : left->_keys) {
        keys.push_back(key);
    }
    keys.push_back(separator);
    for (const auto& key : right->_keys) {
        keys.push_back(key);
    }

    if (!left->_pointers.empty()) {
        pointers.reserve(left->_pointers.size() + right->_pointers.size());
        for (bstree_node* child : left->_pointers) {
            pointers.push_back(child);
        }
        for (bstree_node* child : right->_pointers) {
            pointers.push_back(child);
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::insertIntoSubtree(bstree_node* node, const tree_data_type& data)
{
    const compare& cmp = static_cast<const compare&>(*this);
    size_t keyIndex = lower_index_in_node(node, data.first);

    if (keyIndex < node->_keys.size() && keysEqual(node->_keys[keyIndex].first, data.first, cmp)) {
        return false;
    }

    if (is_leaf_node(node)) {
        node->_keys.insert(node->_keys.begin() + static_cast<std::ptrdiff_t>(keyIndex), data);
        return true;
    }

    bool inserted = insertIntoSubtree(node->_pointers[keyIndex], data);

    if (inserted && node->_pointers[keyIndex]->_keys.size() > maximum_keys_in_node) {
        fixChildOverflow(node, keyIndex);
    }

    return inserted;
}

// Перераспределить ключи между двумя соседними детьми и разделяющим ключом родителя
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::redistributeTwoChildren(bstree_node* parent, size_t left_index)
{
    bstree_node* left = parent->_pointers[left_index];
    bstree_node* right = parent->_pointers[left_index + 1];

    std::vector<tree_data_type> combinedKeys;
    std::vector<bstree_node*> combinedPointers;
    collectTwoChildren(left, parent->_keys[left_index], right, combinedKeys, combinedPointers);

    const size_t leftCount = (combinedKeys.size() - 1) / 2;
    const size_t rightCount = combinedKeys.size() - 1 - leftCount;

    parent->_keys[left_index] = combinedKeys[leftCount];

    createNodeFromList(left, combinedKeys, 0, leftCount, combinedPointers, 0);
    createNodeFromList(right, combinedKeys, leftCount + 1, rightCount, combinedPointers, leftCount + 1);
}

// Разделить двух соседних детей и разделяющий ключ родителя на три узла
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::twoThree(bstree_node* parent, size_t left_index)
{
    bstree_node* left = parent->_pointers[left_index];
    bstree_node* middle = parent->_pointers[left_index + 1];
    bstree_node* right = create_node();

    std::vector<tree_data_type> combinedKeys;
    std::vector<bstree_node*> combinedPointers;
    collectTwoChildren(left, parent->_keys[left_index], middle, combinedKeys, combinedPointers);

    const size_t payload = combinedKeys.size() - 2;
    const size_t base = payload / 3;
    const size_t remainder = payload % 3;

    const size_t firstCount = base;
    const size_t secondCount = base + (remainder == 2 ? 1 : 0);
    const size_t thirdCount = base + (remainder >= 1 ? 1 : 0);

    const size_t firstSeparatorIndex = firstCount;
    const size_t secondSeparatorIndex = firstCount + 1 + secondCount;

    const tree_data_type firstSeparator = combinedKeys[firstSeparatorIndex];
    const tree_data_type secondSeparator = combinedKeys[secondSeparatorIndex];

    createNodeFromList(left, combinedKeys, 0, firstCount, combinedPointers, 0);
    createNodeFromList(middle, combinedKeys, firstSeparatorIndex + 1, secondCount, combinedPointers, firstSeparatorIndex + 1);
    createNodeFromList(right, combinedKeys, secondSeparatorIndex + 1, thirdCount, combinedPointers, secondSeparatorIndex + 1);

    parent->_keys[left_index] = firstSeparator;
    parent->_keys.insert(parent->_keys.begin() + static_cast<std::ptrdiff_t>(left_index + 1), secondSeparator);
    parent->_pointers.insert(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(left_index + 2), right);
}

// Fix переполнения детей после вставки
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::fixChildOverflow(bstree_node* parent, size_t child_index)
{
    // Случай 1 - всё нормально
    if (parent->_pointers[child_index]->_keys.size() <= maximum_keys_in_node) {
        return;
    }

    // Случай 2 - перераспределяем с соседом
    if (child_index > 0 && parent->_pointers[child_index - 1]->_keys.size() < maximum_keys_in_node) {
        redistributeTwoChildren(parent, child_index - 1);
    } else if (child_index + 1 < parent->_pointers.size() && parent->_pointers[child_index + 1]->_keys.size() < maximum_keys_in_node) {
        redistributeTwoChildren(parent, child_index);
    // Случай 3 - делим на 3 узла
    } else if (child_index > 0) {
        twoThree(parent, child_index - 1);
    } else {
        twoThree(parent, child_index);
    }
}

// Переполнение корня
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::splitRoot()
{
    bstree_node* oldRoot = _root;
    bstree_node* newRight = create_node();
    bstree_node* newRoot = create_node();

    std::vector<tree_data_type> keys;
    std::vector<bstree_node*> pointers;
    keys.reserve(oldRoot->_keys.size());

    for (const auto& key : oldRoot->_keys) {
        keys.push_back(key);
    }

    if (!oldRoot->_pointers.empty()) {
        pointers.reserve(oldRoot->_pointers.size());
        for (bstree_node* child : oldRoot->_pointers) {
            pointers.push_back(child);
        }
    }

    const size_t leftCount = t;
    const tree_data_type separator = keys[leftCount];
    const size_t rightCount = keys.size() - leftCount - 1;

    createNodeFromList(oldRoot, keys, 0, leftCount, pointers, 0);
    createNodeFromList(newRight, keys, leftCount + 1, rightCount, pointers, leftCount + 1);

    newRoot->_keys.push_back(separator);
    newRoot->_pointers.push_back(oldRoot);
    newRoot->_pointers.push_back(newRight);

    _root = newRoot;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const compare& cmp, pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(pp_allocator<value_type> alloc, const compare& comp)
{
    static_cast<compare&>(*this) = comp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BS_tree<tkey, tvalue, compare, t>::BS_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;

    for (; begin != end; ++begin) {
        insert(*begin);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;

    for (const auto& item : data) {
        insert(item);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::~BS_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const BS_tree& other)
{
    static_cast<compare&>(*this) = static_cast<const compare&>(other);
    _allocator = other._allocator;
    _root = clone_subtree(other._root);
    _size = other._size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(BS_tree&& other) noexcept
{
    static_cast<compare&>(*this) = static_cast<compare&&>(other);
    _allocator = other._allocator;
    _root = other._root;
    _size = other._size;
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(const BS_tree& other)
{
    if (this == &other) {
        return *this;
    }

    BS_tree copy(other);
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
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(BS_tree&& other) noexcept
{
    if (this == &other) {
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
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::bstree_iterator(
    const std::stack<std::pair<bstree_node**, size_t>>& path,
    size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::reference
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator*() const noexcept
{
    bstree_node* node = _path.empty() ? nullptr : *(_path.top().first);
    return *reinterpret_cast<value_type*>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::pointer
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator->() const noexcept
{
    return &(**this);
}

// Подобно B_Tree (префиксный инкремент)
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator++()
{
    if (_path.empty()) {
        return *this;
    }

    auto pathVector = stackToVector(_path);
    bstree_node* currentNode = *pathVector.back().first;

    // Случай 1 - внутренний узел
    if (_index < currentNode->_keys.size() && !currentNode->_pointers.empty()) {
        bstree_node** childLink = &currentNode->_pointers[_index + 1];
        pathVector.push_back({childLink, _index + 1});
        while (!(*childLink)->_pointers.empty()) {
            childLink = &(*childLink)->_pointers[0];
            pathVector.push_back({childLink, 0});
        }
        _path = vectorToStack(pathVector);
        _index = 0;
        return *this;
    }

    // Случай 2 - после листа идёт следующий лист
    if (_index + 1 < currentNode->_keys.size()) {
        ++_index;
        return *this;
    }

    // Случай 3 - последний ключ у этого узла
    auto originalPath = pathVector;
    while (pathVector.size() > 1) {
        auto childNode = pathVector.back();
        pathVector.pop_back();
        bstree_node* parentNode = *pathVector.back().first;
        if (childNode.second < parentNode->_keys.size()) {
            _path = vectorToStack(pathVector);
            _index = childNode.second;
            return *this;
        }
    }

    // Случай 4 - это конец
    _path = vectorToStack(originalPath);
    _index = currentNode->_keys.size();
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator--()
{
    // Случай 0 - итератор никуда не указывает
    if (_path.empty()) {
        return *this;
    }

    auto pathVector = stackToVector(_path);
    bstree_node* currentNode = *pathVector.back().first;

    // Случай 1 - это конец?
    if (_index == currentNode->_keys.size()) {
        if (!currentNode->_keys.empty()) {
            _index = currentNode->_keys.size() - 1;
        }
        return *this;
    }

    // Случай 2 - внутренний узел
    if (!currentNode->_pointers.empty()) {
        bstree_node** childLink = &currentNode->_pointers[_index];
        pathVector.push_back({childLink, _index});

        while (!(*childLink)->_pointers.empty()) {
            size_t lastChildIndex = (*childLink)->_pointers.size() - 1;
            childLink = &(*childLink)->_pointers[lastChildIndex];
            pathVector.push_back({childLink, lastChildIndex});
        }
        _path = vectorToStack(pathVector);
        _index = (*childLink)->_keys.size() - 1;
        return *this;
    }

    // Случай 3 - лист с запасом
    if (_index > 0) {
        --_index;
        return *this;
    }

    // Случай 4 - первый лист
    while (pathVector.size() > 1) {
        auto childNode = pathVector.back();
        pathVector.pop_back();
        if (childNode.second > 0) {
            _path = vectorToStack(pathVector);
            _index = childNode.second - 1;
            return *this;
        }
    }

    // Случай 5 - это и есть первый элемент
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(
    const std::stack<std::pair<bstree_node* const*, size_t>>& path,
    size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(const bstree_iterator& it) noexcept
{
    auto changeablePath = stackToVector(it._path);
    std::vector<std::pair<bstree_node* const*, size_t>> constPath;
    constPath.reserve(changeablePath.size());
    for (const auto& frame : changeablePath) {
        constPath.push_back({frame.first, frame.second});
    }
    _path = vectorToStack(constPath);
    _index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::reference
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator*() const noexcept
{
    const bstree_node* node = _path.empty() ? nullptr : *(_path.top().first);
    return *reinterpret_cast<const value_type*>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::pointer
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator++()
{
    bstree_iterator temp;
    auto constPath = stackToVector(_path);
    std::vector<std::pair<bstree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());
    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<bstree_node**>(frame.first), frame.second});
    }
    temp = bstree_iterator(vectorToStack(changeablePath), _index);
    ++temp;
    *this = bstree_const_iterator(temp);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator--()
{
    bstree_iterator temp;
    auto constPath = stackToVector(_path);
    std::vector<std::pair<bstree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());

    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<bstree_node**>(frame.first), frame.second});
    }
    temp = bstree_iterator(vectorToStack(changeablePath), _index);
    --temp;
    *this = bstree_const_iterator(temp);

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(
    const std::stack<std::pair<bstree_node**, size_t>>& path,
    size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(const bstree_iterator& it) noexcept
{
    _path = it._path;
    _index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_iterator() const noexcept
{
    return bstree_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::reference
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator*() const noexcept
{
    bstree_iterator temp = static_cast<bstree_iterator>(*this);
    --temp;
    return *temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::pointer
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator++()
{
    bstree_iterator temp = static_cast<bstree_iterator>(*this);
    --temp;
    _path = temp._path;
    _index = temp._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator--()
{
    bstree_iterator temp = static_cast<bstree_iterator>(*this);
    ++temp;
    _path = temp._path;
    _index = temp._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(
    const std::stack<std::pair<bstree_node* const*, size_t>>& path,
    size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(const bstree_reverse_iterator& it) noexcept
{
    auto changeablePath = stackToVector(it._path);
    std::vector<std::pair<bstree_node* const*, size_t>> constPath;
    constPath.reserve(changeablePath.size());

    for (const auto& frame : changeablePath) {
        constPath.push_back({frame.first, frame.second});
    }
    _path = vectorToStack(constPath);
    _index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator() const noexcept
{
    auto constPath = stackToVector(_path);
    std::vector<std::pair<bstree_node**, size_t>> mutablePath;
    mutablePath.reserve(constPath.size());
    for (const auto& frame : constPath) {
        mutablePath.push_back({const_cast<bstree_node**>(frame.first), frame.second});
    }
    auto mutableStack = vectorToStack(mutablePath);
    bstree_iterator it(mutableStack, _index);
    return bstree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::reference
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator*() const noexcept
{
    bstree_const_iterator temp = static_cast<bstree_const_iterator>(*this);
    --temp;
    return *temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::pointer
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator++()
{
    // const_reverse_iterator -> const_iterator -> операция -> iterator -> reverse_iterator -> const_reverse_iterator 
    bstree_const_iterator temp = static_cast<bstree_const_iterator>(*this);
    --temp;

    auto constPath = stackToVector(temp._path);
    std::vector<std::pair<bstree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());

    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<bstree_node**>(frame.first), frame.second});
    }

    auto changeableStack = vectorToStack(changeablePath);
    bstree_iterator it(changeableStack, temp._index);

    *this = bstree_const_reverse_iterator(bstree_reverse_iterator(it));
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self&
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator--()
{
    bstree_const_iterator temp = static_cast<bstree_const_iterator>(*this);
    ++temp;

    auto constPath = stackToVector(temp._path);
    std::vector<std::pair<bstree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());

    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<bstree_node**>(frame.first), frame.second});
    }

    auto changeableStack = vectorToStack(changeablePath);
    bstree_iterator it(changeableStack, temp._index);

    *this = bstree_const_reverse_iterator(bstree_reverse_iterator(it));
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto iterator = find(key);
    if (iterator == end()) {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }

    return (*iterator).second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto iterator = find(key);
    if (iterator == end()) {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }

    return (*iterator).second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    return emplace(std::move(key), tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr) {
        return end();
    }
    return bstree_iterator(make_leftmost_path(), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::end()
{
    if (_root == nullptr) {
        return bstree_iterator();
    }
    auto path = make_rightmost_path();
    return bstree_iterator(path, (*path.top().first)->_keys.size());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::begin() const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::end() const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->end());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin()
{
    return bstree_reverse_iterator(end());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend()
{
    return bstree_reverse_iterator(begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return bstree_const_reverse_iterator(const_cast<BS_tree*>(this)->rbegin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend() const
{
    return bstree_const_reverse_iterator(const_cast<BS_tree*>(this)->rend());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crbegin() const
{
    return rbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crend() const
{
    return rend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }

    const compare& cmp = static_cast<const compare&>(*this);
    std::vector<std::pair<bstree_node**, size_t>> pathVector;
    bstree_node** currentLink = &_root;
    size_t childIndex = 0;

    while (*currentLink != nullptr) {
        bstree_node* currentNode = *currentLink;
        pathVector.push_back({currentLink, childIndex});
        size_t keyIndex = lower_index_in_node(currentNode, key);

        if (keyIndex < currentNode->_keys.size() && keysEqual(currentNode->_keys[keyIndex].first, key, cmp)) {
            return bstree_iterator(vectorToStack(pathVector), keyIndex);
        }

        if (is_leaf_node(currentNode)) {
            return end();
        }

        childIndex = keyIndex;
        currentLink = &currentNode->_pointers[childIndex];
    }

    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->find(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }

    std::vector<std::pair<bstree_node**, size_t>> pathVector;
    std::vector<std::pair<bstree_node**, size_t>> candidatePath;
    bstree_node** currentLink = &_root;
    size_t childIndex = 0;
    size_t candidateIndex = 0;
    bool hasCandidate = false;

    while (*currentLink != nullptr) {
        bstree_node* currentNode = *currentLink;
        pathVector.push_back({currentLink, childIndex});
        size_t keyIndex = lower_index_in_node(currentNode, key);

        if (keyIndex < currentNode->_keys.size()) {
            candidatePath = pathVector;
            candidateIndex = keyIndex;
            hasCandidate = true;
        }

        if (is_leaf_node(currentNode)) {
            break;
        }

        childIndex = keyIndex;
        currentLink = &currentNode->_pointers[childIndex];
    }

    return hasCandidate ? bstree_iterator(vectorToStack(candidatePath), candidateIndex) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->lower_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }

    std::vector<std::pair<bstree_node**, size_t>> pathVector;
    std::vector<std::pair<bstree_node**, size_t>> candidatePath;
    bstree_node** currentLink = &_root;
    size_t childIndex = 0;
    size_t candidateIndex = 0;
    bool hasCandidate = false;

    while (*currentLink != nullptr) {
        bstree_node* currentNode = *currentLink;
        pathVector.push_back({currentLink, childIndex});
        size_t keyIndex = upper_index_in_node(currentNode, key);

        if (keyIndex < currentNode->_keys.size()) {
            candidatePath = pathVector;
            candidateIndex = keyIndex;
            hasCandidate = true;
        }

        if (is_leaf_node(currentNode)) {
            break;
        }

        childIndex = keyIndex;
        currentLink = &currentNode->_pointers[childIndex];
    }

    return hasCandidate ? bstree_iterator(vectorToStack(candidatePath), candidateIndex) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->upper_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    destroy_subtree(_root);
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool>
BS_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data.first, data.second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool>
BS_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data.first), std::move(data.second));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool>
BS_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);

    auto existing = find(data.first);
    if (existing != end()) {
        return {existing, false};
    }

    if (_root == nullptr) {
        _root = create_node();
        _root->_keys.push_back(std::move(data));
        _size = 1;
        return {begin(), true};
    }

    insertIntoSubtree(_root, data);
    if (_root->_keys.size() > maximum_keys_in_node) {
        splitRoot();
    }

    ++_size;
    return {find(data.first), true};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    return emplace_or_assign(data.first, data.second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    return emplace_or_assign(std::move(data.first), std::move(data.second));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto existing = find(data.first);
    if (existing != end()) {
        existing->second = std::move(data.second);
        return existing;
    }
    return emplace(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator pos)
{
    if (pos == end()) {
        return end();
    }

    auto next = pos;
    ++next;
    std::optional<tkey> nextKey;
    if (next != end()) {
        nextKey = next->first;
    }

    if (erase_from_subtree(&_root, pos->first)) {
        --_size;
    }

    return nextKey.has_value() ? lower_bound(*nextKey) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator pos)
{
    return erase(find(pos->first));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator beg, bstree_iterator en)
{
    if (beg == en) {
        return beg;
    }

    std::optional<tkey> stopKey;
    if (en != end()) {
        stopKey = en->first;
    }

    if (!stopKey.has_value()) {
        while (beg != end()) {
            beg = erase(beg);
        }
        return end();
    }

    while (beg != end() && compare_keys(beg->first, *stopKey)) {
        beg = erase(beg);
    }

    return lower_bound(*stopKey);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator beg, bstree_const_iterator en)
{
    if (beg == en) {
        return en == cend() ? end() : lower_bound(en->first);
    }

    std::optional<tkey> stopKey;
    if (en != cend()) {
        stopKey = en->first;
    }

    if (!stopKey.has_value()) {
        while (beg != cend()) {
            beg = erase(beg);
        }
        return end();
    }

    while (beg != cend() && compare_keys(beg->first, *stopKey)) {
        beg = erase(beg);
    }

    return lower_bound(*stopKey);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);
    if (it == end()) {
        return end();
    }
    return erase(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_keys(const tkey& lhs, const tkey& rhs) const
{
    return compare::operator()(lhs, rhs);
}

#pragma endregion

#endif

// Запуск программы: 
// cmake --build build
// ./build/associative_container/indexing_tree/b_star_tree/tests/sys_prog_assctv_cntnr_indxng_tr_b_str_tr_tests 

// 9-ый тест всё ещё осуждаю.