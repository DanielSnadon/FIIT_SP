// #pragma region *
// #pragma endregion

// ПОЛЕЗНАЯ ИНФОРМАЦИЯ В КОНЦЕ ФАЙЛА.

#ifndef SYS_PROG_B_TREE_H
#define SYS_PROG_B_TREE_H

#include <iterator>
#include <utility>
#include <boost/container/static_vector.hpp>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <type_traits>

#pragma region helperFuncs

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

template<typename Key, typename Compare>
bool keysEqual(const Key& left, const Key& right, const Compare& comp)
{
	return !comp(left, right) && !comp(right, left);
}

#pragma endregion
#pragma region given

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class B_tree final : private compare // EBCO
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


    struct btree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<btree_node*, maximum_keys_in_node + 2> _pointers;
        btree_node() noexcept;
    };

    pp_allocator<value_type> _allocator;
    btree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;


private:
    static bool is_leaf_node(const btree_node* node) noexcept;
    btree_node* create_node() const;
    void destroy_node(btree_node* node) const noexcept;
    void destroy_subtree(btree_node* node) noexcept;
    btree_node* clone_subtree(const btree_node* node) const;
    size_t lower_index_in_node(const btree_node* node, const tkey& key) const;
    size_t upper_index_in_node(const btree_node* node, const tkey& key) const;
    std::stack<std::pair<btree_node**, size_t>> make_leftmost_path();
    std::stack<std::pair<btree_node**, size_t>> make_rightmost_path();
    void merge_children(btree_node* parent, size_t key_index);
    void borrow_from_left(btree_node* parent, size_t child_index);
    void borrow_from_right(btree_node* parent, size_t child_index);
    tree_data_type max_key_in_subtree(const btree_node* node) const;
    tree_data_type min_key_in_subtree(const btree_node* node) const;
    bool erase_from_subtree(btree_node** node_link, const tkey& key);

public:

    // region constructors declaration

    explicit B_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit B_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit B_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    B_tree(const B_tree& other);

    B_tree(B_tree&& other) noexcept;

    B_tree& operator=(const B_tree& other);

    B_tree& operator=(B_tree&& other) noexcept;

    ~B_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class btree_iterator;
    class btree_reverse_iterator;
    class btree_const_iterator;
    class btree_const_reverse_iterator;

    class btree_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

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

        explicit btree_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);

    };

    class btree_const_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_iterator;
        friend class btree_const_reverse_iterator;

        btree_const_iterator(const btree_iterator& it) noexcept;

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

        explicit btree_const_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    class btree_reverse_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_reverse_iterator;

        friend class B_tree;
        friend class btree_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        btree_reverse_iterator(const btree_iterator& it) noexcept;
        operator btree_iterator() const noexcept;

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

        explicit btree_reverse_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);
    };

    class btree_const_reverse_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_reverse_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_iterator;

        btree_const_reverse_iterator(const btree_reverse_iterator& it) noexcept;
        operator btree_const_iterator() const noexcept;

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

        explicit btree_const_reverse_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;
    friend class btree_reverse_iterator;
    friend class btree_const_reverse_iterator;

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

    btree_iterator begin();
    btree_iterator end();

    btree_const_iterator begin() const;
    btree_const_iterator end() const;

    btree_const_iterator cbegin() const;
    btree_const_iterator cend() const;

    btree_reverse_iterator rbegin();
    btree_reverse_iterator rend();

    btree_const_reverse_iterator rbegin() const;
    btree_const_reverse_iterator rend() const;

    btree_const_reverse_iterator crbegin() const;
    btree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    btree_iterator find(const tkey& key);
    btree_const_iterator find(const tkey& key) const;

    btree_iterator lower_bound(const tkey& key);
    btree_const_iterator lower_bound(const tkey& key) const;

    btree_iterator upper_bound(const tkey& key);
    btree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<btree_iterator, bool> insert(const tree_data_type& data);
    std::pair<btree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<btree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    btree_iterator insert_or_assign(const tree_data_type& data);
    btree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    btree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    btree_iterator erase(btree_iterator pos);
    btree_iterator erase(btree_const_iterator pos);

    btree_iterator erase(btree_iterator beg, btree_iterator en);
    btree_iterator erase(btree_const_iterator beg, btree_const_iterator en);


    btree_iterator erase(const tkey& key);

    // endregion modifiers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
B_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_pairs(const B_tree::tree_data_type &lhs,
                                                     const B_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

#pragma endregion
#pragma region taskItself
// Непосредственно задача

// Already key/value constructorы существуют
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_node::btree_node() noexcept {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename B_tree<tkey, tvalue, compare, t>::value_type> B_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
{
    return _allocator;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::is_leaf_node(const btree_node* node) noexcept
{
    return node == nullptr || node->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_node* B_tree<tkey, tvalue, compare, t>::create_node() const
{
	// Динамической памяти нет.
    return new btree_node();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::destroy_node(btree_node* node) const noexcept
{
    delete node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::destroy_subtree(btree_node* node) noexcept
{
    if (node == nullptr) {
        return;
    }

    for (btree_node* child : node->_pointers) {
        destroy_subtree(child);
    }

    destroy_node(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_node* B_tree<tkey, tvalue, compare, t>::clone_subtree(const btree_node* node) const
{
    if (node == nullptr) {
        return nullptr;
    }

    btree_node* copy = create_node();
    try
    {
        copy->_keys = node->_keys;
        for (btree_node* child : node->_pointers) {
            copy->_pointers.push_back(clone_subtree(child));
        }
    } catch (...) {
        destroy_subtree(copy);
        throw;
    }

    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::lower_index_in_node(const btree_node* node, const tkey& key) const
{
    size_t index = 0;
    while (index < node->_keys.size() && compare_keys(node->_keys[index].first, key)) {
        ++index;
    }
    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::upper_index_in_node(const btree_node* node, const tkey& key) const
{
    // Ищем первую позицию, на которой ключ в узле строго больше искомого.
    size_t index = 0;
    while (index < node->_keys.size() && !compare_keys(key, node->_keys[index].first)) {
        ++index;
    }
    return index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::stack<std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_node**, size_t>>
B_tree<tkey, tvalue, compare, t>::make_leftmost_path()
{
    std::stack<std::pair<btree_node**, size_t>> path;
    if (_root == nullptr) {
        return path;
    }

    btree_node** currentLink = &_root;
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
std::stack<std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_node**, size_t>>
B_tree<tkey, tvalue, compare, t>::make_rightmost_path()
{
    std::stack<std::pair<btree_node**, size_t>> path;
    if (_root == nullptr) {
        return path;
    }

    btree_node** currentLink = &_root;
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
void B_tree<tkey, tvalue, compare, t>::merge_children(btree_node* parent, size_t key_index)
{
    btree_node* leftChild = parent->_pointers[key_index];
    btree_node* rightChild = parent->_pointers[key_index + 1];

    leftChild->_keys.push_back(std::move(parent->_keys[key_index]));
    for (auto& keyValue : rightChild->_keys) {
        leftChild->_keys.push_back(std::move(keyValue));
    }
    for (btree_node* child : rightChild->_pointers) {
        leftChild->_pointers.push_back(child);
    }

    parent->_keys.erase(parent->_keys.begin() + static_cast<std::ptrdiff_t>(key_index));
    parent->_pointers.erase(parent->_pointers.begin() + static_cast<std::ptrdiff_t>(key_index + 1));

    destroy_node(rightChild);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::borrow_from_left(btree_node* parent, size_t child_index)
{
    btree_node* leftSibling = parent->_pointers[child_index - 1];
    btree_node* child = parent->_pointers[child_index];

    child->_keys.insert(child->_keys.begin(), std::move(parent->_keys[child_index - 1]));
    parent->_keys[child_index - 1] = std::move(leftSibling->_keys.back());
    leftSibling->_keys.pop_back();

    if (!is_leaf_node(leftSibling)) {
        child->_pointers.insert(child->_pointers.begin(), leftSibling->_pointers.back());
        leftSibling->_pointers.pop_back();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::borrow_from_right(btree_node* parent, size_t child_index)
{
    btree_node* child = parent->_pointers[child_index];
    btree_node* rightSibling = parent->_pointers[child_index + 1];

    child->_keys.push_back(std::move(parent->_keys[child_index]));
    parent->_keys[child_index] = std::move(rightSibling->_keys.front());
    rightSibling->_keys.erase(rightSibling->_keys.begin());

    if (!is_leaf_node(rightSibling)) {
        child->_pointers.push_back(rightSibling->_pointers.front());
        rightSibling->_pointers.erase(rightSibling->_pointers.begin());
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::tree_data_type
B_tree<tkey, tvalue, compare, t>::max_key_in_subtree(const btree_node* node) const
{
    while (!is_leaf_node(node)) {
        node = node->_pointers.back();
    }
    return node->_keys.back();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::tree_data_type
B_tree<tkey, tvalue, compare, t>::min_key_in_subtree(const btree_node* node) const
{
    while (!is_leaf_node(node)) {
        node = node->_pointers.front();
    }
    return node->_keys.front();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::erase_from_subtree(btree_node** node_link, const tkey& key)
{
    btree_node* node = *node_link;
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
        btree_node* child = node->_pointers[childIndex];
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
            btree_node* newRoot = node->_pointers.front();
            node->_pointers.clear();
            destroy_node(node);
            _root = newRoot;
            *node_link = newRoot;
        }
    }

    return true;
}

// region constructors implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        const compare& cmp,
        pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        pp_allocator<value_type> alloc,\
        const compare& comp)
{
    static_cast<compare&>(*this) = comp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
B_tree<tkey, tvalue, compare, t>::B_tree(
        iterator begin,
        iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc)
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
B_tree<tkey, tvalue, compare, t>::B_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc)
{
    static_cast<compare&>(*this) = cmp;
    _allocator = alloc;
    _root = nullptr;
    _size = 0;

    for (const auto& item : data) {
        insert(item);
    }
}

// endregion constructors implementation

// region five implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree& other)
{
    static_cast<compare&>(*this) = static_cast<const compare&>(other);
    _allocator = other._allocator;
    _root = clone_subtree(other._root);
    _size = other._size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(const B_tree& other)
{
    if (this == &other) {
        return *this;
    }

    B_tree copy(other);
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
B_tree<tkey, tvalue, compare, t>::B_tree(B_tree&& other) noexcept
{
    static_cast<compare&>(*this) = static_cast<compare&&>(other);
    _allocator = other._allocator;
    _root = other._root;
    _size = other._size;
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(B_tree&& other) noexcept
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

// endregion five implementation

// region iterators implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept
{
    btree_node* node = _path.empty() ? nullptr : *(_path.top().first);
    return *reinterpret_cast<value_type*>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++()
{
	// Случай 0 - итератор никуда не указывает
    if (_path.empty()) {
        return *this;
    }

    auto pathVector = stackToVector(_path);
    btree_node* currentNode = *pathVector.back().first;

    // Случай 1 - внутренний узел
    if (_index < currentNode->_keys.size() && !currentNode->_pointers.empty()) {
        btree_node** childLink = &currentNode->_pointers[_index + 1];
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
        btree_node* parentNode = *pathVector.back().first;
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
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--()
{
	// Случай 0 - итератор никуда не указывает
    if (_path.empty()) {
        return *this;
    }
    
    auto pathVector = stackToVector(_path);
    btree_node* currentNode = *pathVector.back().first;
    
    // Случай 1 - это конец?
    if (_index == currentNode->_keys.size()) {
        if (!currentNode->_keys.empty()) {
            _index = currentNode->_keys.size() - 1;
        }
        return *this;
    }

    // Случай 2 - внутренний узел
    if (!currentNode->_pointers.empty()) {
        btree_node** childLink = &currentNode->_pointers[_index];
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
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const btree_iterator& it) noexcept
{
    auto changeablePath = stackToVector(it._path);
    std::vector<std::pair<btree_node* const*, size_t>> constPath;
    constPath.reserve(changeablePath.size());

    for (const auto& frame : changeablePath) {
        constPath.push_back({frame.first, frame.second});
    }

    _path = vectorToStack(constPath);
    _index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*() const noexcept
{
    const btree_node* node = _path.empty() ? nullptr : *(_path.top().first);
    return *reinterpret_cast<const value_type*>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++()
{
    btree_iterator temp;
    auto constPath = stackToVector(_path);
    std::vector<std::pair<btree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());
    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<btree_node**>(frame.first), frame.second});
    }
    temp = btree_iterator(vectorToStack(changeablePath), _index);
    ++temp;
    *this = btree_const_iterator(temp);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()
{
    btree_iterator temp;
    auto constPath = stackToVector(_path);
    std::vector<std::pair<btree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());

    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<btree_node**>(frame.first), frame.second});
    }

    temp = btree_iterator(vectorToStack(changeablePath), _index);
    --temp;
    *this = btree_const_iterator(temp);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const btree_iterator& it) noexcept
{
    _path = it._path;
    _index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_iterator() const noexcept
{
    return btree_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*() const noexcept
{
    btree_iterator temp = static_cast<btree_iterator>(*this);
    --temp;
    return *temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()
{
    btree_iterator temp = static_cast<btree_iterator>(*this);
    --temp;
    _path = temp._path;
    _index = temp._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()
{
    btree_iterator temp = static_cast<btree_iterator>(*this);
    ++temp;
    _path = temp._path;
    _index = temp._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index)
{
    _path = path;
    _index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const btree_reverse_iterator& it) noexcept
{
    auto changeablePath = stackToVector(it._path);
    std::vector<std::pair<btree_node* const*, size_t>> constPath;
    constPath.reserve(changeablePath.size());

    for (const auto& frame : changeablePath) {
        constPath.push_back({frame.first, frame.second});
    }
    
    _path = vectorToStack(constPath);
    _index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator() const noexcept
{
    auto constPath = stackToVector(_path);

    std::vector<std::pair<btree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());

    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<btree_node**>(frame.first), frame.second});
    }

    auto changeableStack = vectorToStack(changeablePath);
    btree_iterator it(changeableStack, _index);
    return btree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*() const noexcept
{
    btree_const_iterator temp = static_cast<btree_const_iterator>(*this);
    --temp;
    return *temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++()
{
    btree_const_iterator temp = static_cast<btree_const_iterator>(*this);
    --temp;
    
    auto constPath = stackToVector(temp._path);

    std::vector<std::pair<btree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());
	// const_reverse_iterator -> const_iterator -> операция -> iterator -> reverse_iterator -> const_reverse_iterator
    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<btree_node**>(frame.first), frame.second});
    }

    auto changeableStack = vectorToStack(changeablePath);
    btree_iterator it(changeableStack, temp._index);

    *this = btree_const_reverse_iterator(btree_reverse_iterator(it));
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(int)
{
    self copy(*this);
    ++(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--()
{
    btree_const_iterator temp = static_cast<btree_const_iterator>(*this);
    ++temp;

    auto constPath = stackToVector(temp._path);

    std::vector<std::pair<btree_node**, size_t>> changeablePath;
    changeablePath.reserve(constPath.size());
	// const_reverse_iterator -> const_iterator -> операция -> iterator -> reverse_iterator -> const_reverse_iterator
    for (const auto& frame : constPath) {
        changeablePath.push_back({const_cast<btree_node**>(frame.first), frame.second});
    }

    auto changeableStack = vectorToStack(changeablePath);
    btree_iterator it(changeableStack, temp._index);

    *this = btree_const_reverse_iterator(btree_reverse_iterator(it));
    return *this;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(int)
{
    self copy(*this);
    --(*this);
    return copy;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && equalStacks(_path, other._path);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::index() const noexcept
{
    return _index;
}

// endregion iterators implementation

// region element access implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto iterator = find(key);
    if (iterator == end()) {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }
    return (*iterator).second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto iterator = find(key);
    if (iterator == end()) {
        throw std::out_of_range("Ошибка: искомый ключ не найден");
    }
    return (*iterator).second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    return emplace(key, tvalue()).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    return emplace(std::move(key), tvalue()).first->second;
}

// endregion element access implementation

// region iterator begins implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr) {
        return end();
    }
    return btree_iterator(make_leftmost_path(), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::end()
{
    if (_root == nullptr) {
        return btree_iterator();
    }
    auto path = make_rightmost_path();
    return btree_iterator(path, (*path.top().first)->_keys.size());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::begin() const
{
    return btree_const_iterator(const_cast<B_tree*>(this)->begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::end() const
{
    return btree_const_iterator(const_cast<B_tree*>(this)->end());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin()
{
    return btree_reverse_iterator(end());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend()
{
    return btree_reverse_iterator(begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return btree_const_reverse_iterator(const_cast<B_tree*>(this)->rbegin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend() const
{
    return btree_const_reverse_iterator(const_cast<B_tree*>(this)->rend());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crbegin() const
{
    return rbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crend() const
{
    return rend();
}

// endregion iterator begins implementation

// region lookup implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }

    const compare& cmp = static_cast<const compare&>(*this);
    std::vector<std::pair<btree_node**, size_t>> pathVector;
    btree_node** currentLink = &_root;
    size_t childIndex = 0;

    while (*currentLink != nullptr) {
        btree_node* currentNode = *currentLink;
        pathVector.push_back({currentLink, childIndex});
        size_t keyIndex = lower_index_in_node(currentNode, key);

        if (keyIndex < currentNode->_keys.size() && keysEqual(currentNode->_keys[keyIndex].first, key, cmp)) {
            return btree_iterator(vectorToStack(pathVector), keyIndex);
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
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    return btree_const_iterator(const_cast<B_tree*>(this)->find(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }

    const compare& cmp = static_cast<const compare&>(*this);
    std::vector<std::pair<btree_node**, size_t>> pathVector;
    std::vector<std::pair<btree_node**, size_t>> bestPath;
    size_t bestIndex = 0;
    bool hasBest = false;

    btree_node** currentLink = &_root;
    size_t childIndex = 0;
    while (*currentLink != nullptr) {
        btree_node* currentNode = *currentLink;
        pathVector.push_back({currentLink, childIndex});
        size_t keyIndex = lower_index_in_node(currentNode, key);

        if (keyIndex < currentNode->_keys.size()) {
            bestPath = pathVector;
            bestIndex = keyIndex;
            hasBest = true;
            if (keysEqual(currentNode->_keys[keyIndex].first, key, cmp)) {
                return btree_iterator(vectorToStack(bestPath), bestIndex);
            }
        }

        if (is_leaf_node(currentNode)) {
            break;
        }

        childIndex = keyIndex;
        currentLink = &currentNode->_pointers[childIndex];
    }

    return hasBest ? btree_iterator(vectorToStack(bestPath), bestIndex) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    return btree_const_iterator(const_cast<B_tree*>(this)->lower_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }

    std::vector<std::pair<btree_node**, size_t>> pathVector;
    std::vector<std::pair<btree_node**, size_t>> bestPath;
    size_t bestIndex = 0;
    bool hasBest = false;

    btree_node** currentLink = &_root;
    size_t childIndex = 0;
    while (*currentLink != nullptr) {
        btree_node* currentNode = *currentLink;
        pathVector.push_back({currentLink, childIndex});
        size_t keyIndex = upper_index_in_node(currentNode, key);

        if (keyIndex < currentNode->_keys.size()) {
            bestPath = pathVector;
            bestIndex = keyIndex;
            hasBest = true;
        }

        if (is_leaf_node(currentNode)) {
            break;
        }

        childIndex = upper_index_in_node(currentNode, key);
        currentLink = &currentNode->_pointers[childIndex];
    }

    return hasBest ? btree_iterator(vectorToStack(bestPath), bestIndex) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    return btree_const_iterator(const_cast<B_tree*>(this)->upper_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion lookup implementation

// region modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    destroy_subtree(_root);
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data.first, data.second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data.first), std::move(data.second));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    tkey insertedKey = data.first;
    const compare& cmp = static_cast<const compare&>(*this);
    
    // Непосредственно вставка
    
    if (_root == nullptr) {
        _root = create_node();
        _root->_keys.push_back(std::move(data));
        _size = 1;
        return {begin(), true};
    }
    
    std::vector<std::pair<btree_node**, size_t>> pathVector;
    btree_node** currentLink = &_root;
    size_t childIndex = 0;

    while (true) {
        btree_node* currentNode = *currentLink;
        pathVector.push_back({currentLink, childIndex});
        size_t keyIndex = lower_index_in_node(currentNode, data.first);

        if (keyIndex < currentNode->_keys.size() && keysEqual(currentNode->_keys[keyIndex].first, data.first, cmp)) {
            return {btree_iterator(vectorToStack(pathVector), keyIndex), false};
        }

        if (is_leaf_node(currentNode)) {
            currentNode->_keys.insert(currentNode->_keys.begin() + static_cast<std::ptrdiff_t>(keyIndex), std::move(data));
            ++_size;
            
            // Работа над переполнением
            
            while (currentNode->_keys.size() > maximum_keys_in_node) {
                size_t middleIndex = t;
                tree_data_type promotedKey = std::move(currentNode->_keys[middleIndex]);
                btree_node* rightNode = create_node();

                for (size_t i = middleIndex + 1; i < currentNode->_keys.size(); ++i) {
                    rightNode->_keys.push_back(std::move(currentNode->_keys[i]));
                }
                currentNode->_keys.erase(currentNode->_keys.begin() + static_cast<std::ptrdiff_t>(middleIndex), currentNode->_keys.end());

                if (!is_leaf_node(currentNode)) {
                    for (size_t i = middleIndex + 1; i < currentNode->_pointers.size(); ++i) {
                        rightNode->_pointers.push_back(currentNode->_pointers[i]);
                    }
                    currentNode->_pointers.erase(currentNode->_pointers.begin() + static_cast<std::ptrdiff_t>(middleIndex + 1), currentNode->_pointers.end());
                }

                if (pathVector.size() == 1) {
                    btree_node* newRoot = create_node();
                    newRoot->_keys.push_back(std::move(promotedKey));
                    newRoot->_pointers.push_back(currentNode);
                    newRoot->_pointers.push_back(rightNode);
                    _root = newRoot;
                    break;
                }

                size_t childPositionInParent = pathVector.back().second;
                pathVector.pop_back();
                btree_node* parentNode = *pathVector.back().first;
                parentNode->_keys.insert(parentNode->_keys.begin() + static_cast<std::ptrdiff_t>(childPositionInParent), std::move(promotedKey));
                parentNode->_pointers.insert(parentNode->_pointers.begin() + static_cast<std::ptrdiff_t>(childPositionInParent + 1), rightNode);
                currentNode = parentNode;
            }

            return {find(insertedKey), true};
        }

        childIndex = keyIndex;
        currentLink = &currentNode->_pointers[childIndex];
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto result = emplace(data.first, data.second);

    if (!result.second) {
        result.first->second = data.second;
    }

    return result.first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    tkey keyCopy = data.first;
    auto result = emplace(std::move(data.first), std::move(data.second));
    
    if (!result.second) {
        result.first->second = std::move(data.second);
    }
    return result.first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto result = emplace(data.first, data.second);
    if (!result.second) {
        result.first->second = std::move(data.second);
    }
    return result.first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)
{
    if (pos == end()) {
        return end();
    }

    tkey key = pos->first;
    auto nextIterator = pos;
    ++nextIterator;
    bool hasNext = nextIterator != end();
    tkey nextKey{};
    if (hasNext) {
        nextKey = nextIterator->first;
    }

    if (erase_from_subtree(&_root, key)) {
        --_size;
    }

    return hasNext ? find(nextKey) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)
{
    return erase(find(pos->first));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en)
{
    std::vector<tkey> keysToErase;
    for (auto iterator = beg; iterator != en; ++iterator) {
        keysToErase.push_back(iterator->first);
    }

    for (const auto& key : keysToErase) {
        erase(key);
    }

    return en == end() ? end() : find(en->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, btree_const_iterator en)
{
    std::vector<tkey> keysToErase;
    for (auto iterator = beg; iterator != en; ++iterator) {
        keysToErase.push_back(iterator->first);
    }

    for (const auto& key : keysToErase) {
        erase(key);
    }

    return en == end() ? end() : find(en->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto iterator = find(key);
    if (iterator == end()) {
        return end();
    }
    return erase(iterator);
}

// endregion modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_pairs(const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &lhs,
                   const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs)
{
    return compare()(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_keys(const tkey &lhs, const tkey &rhs)
{
    return compare()(lhs, rhs);
}

#pragma endregion

#endif

// Алгоритмические сложности:
// Вставка - O(t log n)
// Удаление - O(t log n)
// Поиск - O(t log n)

// Запуск программы: 
// cmake --build build
// ./build/associative_container/indexing_tree/b_tree/tests/sys_prog_assctv_cntnr_indxng_tr_b_tr_tests

// Тест №9 имеет некорректные данные. Там демонстрируется проход от начала до конца, но ожидаемое значение не логично. 
// Корректные по идее должны быть: 
/* 
std::vector<B_tree<int, std::string>::value_type> expected_result =
{
    {1,   "a"},
    {2,   "b"},
    {3,   "d"},
    {4,   "e"},
    {15,  "c"},
    {24,  "g"},
    {45,  "k"},
    {100, "f"},
    {101, "j"},
    {193, "l"},
    {456, "h"},
    {534, "m"},
};
*/