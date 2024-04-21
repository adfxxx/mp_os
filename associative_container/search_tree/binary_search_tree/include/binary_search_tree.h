#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H

#include <list>
#include <stack>
#include <vector>
#include <logger.h>
#include <logger_guardant.h>
#include <allocator.h>
#include <allocator_guardant.h>
#include <not_implemented.h>
#include <search_tree.h>
#include <queue>
#include <associative_container.h>

template<typename tkey, typename tvalue>
class binary_search_tree:
    public search_tree<tkey, tvalue>
{

protected:
    
    struct node
    {
    
    public:
        
        tkey key;
        tvalue value;
        node *left_subtree;
        node *right_subtree;
    
    public:
        
        explicit node(tkey const &key, tvalue const &value);
        
        explicit node(tkey const &key, tvalue &&value);
    };

public:
    
    // region iterators definition

    struct iterator_data
    {
        public:
            
            unsigned int depth;
            tkey key;
            tvalue value;

        private:

            bool _is_state_initialized;

        public:

            inline tkey const &get_key() const noexcept
            {
                if(is_state_initialized()){
                    return key;
                }
                throw std::logic_error("Binary_search_tree: state is not initialized.\n");
            }

            inline tvalue const &get_value() const noexcept
            {
                if(is_state_initialized()){
                    return value;
                }
                throw std::logic_error("Binary_search_tree: state is not initialized.\n");
            }

            inline bool is_state_initialized() const noexcept
            {
                return _is_state_initialized;
            }

    public:

        explicit iterator_data():
            _is_state_initialized(false)
        {
        }

        explicit iterator_data(unsigned int depth, tkey const &key, tvalue const &value):
            depth(depth), key(key), value(value),
            _is_state_initialized(true)
        {
        }

        explicit iterator_data(unsigned int depth, tkey const &key, tvalue &&value):
            depth(depth), key(key), value(std::move(value)),
            _is_state_initialized(true)
        {
        }
    };
public:
    iterator_data *construct_iterator_data(unsigned int depth, tkey const &key, tvalue const &value) const
    {
        this->trace_with_guard("Binary_search_tree: start constructing.\n");

        iterator_data *data = nullptr;

        try{
            data = reinterpret_cast<iterator_data*>(this->allocate_with_guard(sizeof(iterator_data),1));
            allocator::construct(data, depth, key, value);
        }
        catch(const std::exception &ex){
            this->error_with_guard("Binary_search_tree: failed to allocate data.\n");
            throw ex;
        }
        this->trace_with_guard("Binary_search_tree: end constructing.\n");
        return data;
    }

    void clear_iterator_data(iterator_data *data) const
    {
        allocator::destruct(data);
        this->deallocate_with_guard(data);
    }
    
private:
    void clear(node *&subtree_root){
        this->trace_with_guard("Binary_search_tree: start node deletion.\n");

        if(subtree_root == nullptr){
            return;
        }
        clear(subtree_root->left_subtree);
        clear(subtree_root->right_subtree);
        subtree_root->~node();
        this->deallocate_with_guard(subtree_root);
        subtree_root = nullptr;

        this->trace_with_guard("Binary_search_tree: end node deletion.\n");
    }

    node *copy(node const *subtree_root){
        this->trace_with_guard("Binary_search_tree: start copy node.\n");
        if(subtree_root == nullptr){
            return nullptr;
        }

        node *subtree_copy = reinterpret_cast<node*>(this->allocate_with_guard(sizeof(node),1));

        allocator::construct(subtree_copy, subtree_root->key, subtree_root->value);

        subtree_copy->left_subtree = copy(subtree_root->left_subtree);
        subtree_copy->right_subtree = copy(subtree_root->right_subtree);

        this->trace_with_guard("Binary_search_tree: end copy node.\n");
        return subtree_copy;
    }

protected:
    void prefix_path(unsigned int depth, std::queue<typename binary_search_tree<tkey, tvalue>::iterator_data*> &path, binary_search_tree<tkey, tvalue>::node *ptr) const
    {
        if(ptr == nullptr){
            return;
        }

        path.push(this->construct_iterator_data(depth, ptr->key, ptr->value));
        if(ptr->left_subtree != nullptr){
            prefix_path(depth+1, path, ptr->left_subtree);
        }
        if(ptr->right_subtree != nullptr){
            prefix_path(depth+1, path, ptr->right_subtree);
        }
    }

    void infix_path(unsigned int depth, std::queue<typename binary_search_tree<tkey, tvalue>::iterator_data*> &path, binary_search_tree<tkey, tvalue>::node *ptr) const
    {
        if(ptr == nullptr){
            return;
        }

        if(ptr->left_subtree != nullptr){
            infix_path(depth+1, path, ptr->left_subtree);
        }
        path.push(this->construct_iterator_data(depth, ptr->key, ptr->value));
        if(ptr->right_subtree != nullptr){
            infix_path(depth+1, path, ptr->right_subtree);
        }
    }


    void postfix_path(unsigned int depth, std::queue<typename binary_search_tree<tkey, tvalue>::iterator_data*> &path, binary_search_tree<tkey, tvalue>::node *ptr) const
    {
        if(ptr == nullptr){
            return;
        }

        if(ptr->left_subtree != nullptr){
            postfix_path(depth+1, path, ptr->left_subtree);
        }
        if(ptr->right_subtree != nullptr){
            postfix_path(depth+1, path, ptr->right_subtree);
        }
        path.push(this->construct_iterator_data(depth, ptr->key, ptr->value));
    }

    void clear_path(std::queue<typename binary_search_tree<tkey, tvalue>::iterator_data*> &path) const
    {
        iterator_data *data;
        while(!path.empty()){
            data = path.front();
            path.pop();
            allocator::destruct(data);
            this->deallocate_with_guard(data);
        }
    }

public:
    std::queue<iterator_data*>copy_path(std::queue<iterator_data*> path)
    {
        iterator_data *data;
        std::queue<iterator_data*> new_path;
        while(!path.empty()){
            data = path.front();
            path.pop();
            new_path.push(construct_iterator_data(data->depth, data->key, data->value));
        }
        return new_path;
    }

    void iterator_copy(iterator_data *&ptr, iterator_data *&other_ptr, binary_search_tree<tkey, tvalue> const *&tree, binary_search_tree<tkey, tvalue> const *&other_tree,
    std::queue<iterator_data*> &path, std::queue<iterator_data*> &other_path) const
    {
        if(!path.empty()){
            tree->clear_path(path);
        }

        ptr = nullptr;
        tree = nullptr;

        tree = other_tree;
        path = this->copy_path(other_path);
        ptr = path.front();
    }

    void iterator_move(iterator_data *&ptr, iterator_data *&other_ptr, binary_search_tree<tkey, tvalue> const *&tree, binary_search_tree<tkey, tvalue> const *&other_tree,
    std::queue<iterator_data*> &path, std::queue<iterator_data*> &other_path) const
    {
        if(!path.empty()){
            tree->clear_path(path);
        }
        ptr = nullptr;
        tree = nullptr;

        tree = other_tree;
        ptr = other_ptr;
        path = other_path;

        while(!other_path.empty()){
            path.pop();
        }
    }

public:
    
    class prefix_iterator final
    {
    
    public:
        
        explicit prefix_iterator(const binary_search_tree<tkey,tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);

    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;
    
    public:
        
        bool operator==(prefix_iterator const &other) const noexcept;
        
        bool operator!=(prefix_iterator const &other) const noexcept;
        
        prefix_iterator &operator++();
        
        prefix_iterator const operator++(int not_used);
        
        iterator_data *operator*() const;

        ~prefix_iterator()
        {
            _tree->clear_path(_path);
        }

        prefix_iterator(prefix_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_iterator(prefix_iterator &&other) noexcept
        {
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_iterator &operator=(prefix_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        prefix_iterator &operator=(prefix_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class prefix_const_iterator final
    {
    
    public:
        
        explicit prefix_const_iterator(const binary_search_tree<tkey,tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;
    
    public:
        
        bool operator==(prefix_const_iterator const &other) const noexcept;
        
        bool operator!=(prefix_const_iterator const &other) const noexcept;
        
        prefix_const_iterator &operator++();
        
        prefix_const_iterator const operator++(int not_used);
        
        iterator_data const *operator*() const;

        ~prefix_const_iterator()
        {
            _tree->clear_path(_path);
        }

        prefix_const_iterator(prefix_const_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_const_iterator(prefix_const_iterator &&other) noexcept
        {
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_const_iterator &operator=(prefix_const_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        prefix_const_iterator &operator=(prefix_const_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class prefix_reverse_iterator final: protected allocator_guardant
    {
    
    public:
        
        explicit prefix_reverse_iterator(const binary_search_tree<tkey,tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);

    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;
    
    public:
        
        bool operator==(prefix_reverse_iterator const &other) const noexcept;
        
        bool operator!=(prefix_reverse_iterator const &other) const noexcept;
        
        prefix_reverse_iterator &operator++();
        
        prefix_reverse_iterator const operator++(int not_used);
        
        iterator_data *operator*() const;

        ~prefix_reverse_iterator()
        {
            _tree->clear_path(_path);
        }

        prefix_reverse_iterator(prefix_reverse_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_reverse_iterator(prefix_reverse_iterator &&other) noexcept
        {
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_reverse_iterator &operator=(prefix_reverse_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        prefix_reverse_iterator &operator=(prefix_reverse_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class prefix_const_reverse_iterator final
    {

    public:
        
        explicit prefix_const_reverse_iterator(const binary_search_tree<tkey,tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);

    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;
    
    public:
        
        bool operator==(prefix_const_reverse_iterator const &other) const noexcept;
        
        bool operator!=(prefix_const_reverse_iterator const &other) const noexcept;
        
        prefix_const_reverse_iterator &operator++();
        
        prefix_const_reverse_iterator const operator++(int not_used);
        
        iterator_data const *operator*() const;
        
        ~prefix_const_reverse_iterator()
        {
            _tree->clear_path(_path);
        }

        prefix_const_reverse_iterator(prefix_const_reverse_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_const_reverse_iterator(prefix_const_reverse_iterator &&other) noexcept
        {
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        prefix_const_reverse_iterator &operator=(prefix_const_reverse_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        prefix_const_reverse_iterator &operator=(prefix_const_reverse_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class infix_iterator final
    {

    public:
        
        explicit infix_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);

    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;
    
    public:
        
        bool operator==(infix_iterator const &other) const noexcept;
        
        bool operator!=(infix_iterator const &other) const noexcept;
        
        infix_iterator &operator++();
        
        infix_iterator const operator++(int not_used);
        
        iterator_data *operator*() const;

        ~infix_iterator()
        {
            _tree->clear_path(_path);
        }

        infix_iterator(infix_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_iterator(infix_iterator &&other) noexcept
        {
            if(!_path.empty()){
                _tree->clear_path(_path);
            }
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_iterator &operator=(infix_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        infix_iterator &operator=(infix_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class infix_const_iterator final
    {
    
    public:
        
        explicit infix_const_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;

    public:
        
        bool operator==(infix_const_iterator const &other) const noexcept;
        
        bool operator!=(infix_const_iterator const &other) const noexcept;
        
        infix_const_iterator &operator++();
        
        infix_const_iterator const operator++(int not_used);
        
        iterator_data const *operator*() const;

        ~infix_const_iterator()
        {
            _tree->clear_path(_path);
        }

        infix_const_iterator(infix_const_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_const_iterator(infix_const_iterator &&other) noexcept
        {
            if(!_path.empty()){
                _tree->clear_path(_path);
            }
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_const_iterator &operator=(infix_const_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        infix_const_iterator &operator=(infix_const_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class infix_reverse_iterator final
    {
    
    public:
        
        explicit infix_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;

    public:
        
        bool operator==(infix_reverse_iterator const &other) const noexcept;
        
        bool operator!=(infix_reverse_iterator const &other) const noexcept;
        
        infix_reverse_iterator &operator++();
        
        infix_reverse_iterator const operator++(int not_used);
        
        iterator_data *operator*() const;

        ~infix_reverse_iterator()
        {
            _tree->clear_path(_path);
        }

        infix_reverse_iterator(infix_reverse_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_reverse_iterator(infix_reverse_iterator &&other) noexcept
        {
            if(!_path.empty()){
                clear_path(_path);
            }
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_reverse_iterator &operator=(infix_reverse_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        infix_reverse_iterator &operator=(infix_reverse_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class infix_const_reverse_iterator final
    {
    
    public:
        
        explicit infix_const_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;

    public:
        
        bool operator==(infix_const_reverse_iterator const &other) const noexcept;
        
        bool operator!=(infix_const_reverse_iterator const &other) const noexcept;
        
        infix_const_reverse_iterator &operator++();
        
        infix_const_reverse_iterator const operator++(int not_used);
        
        iterator_data const *operator*() const;
        
        ~infix_const_reverse_iterator()
        {
            _tree->clear_path(_path);
        }

        infix_const_reverse_iterator(infix_const_reverse_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_const_reverse_iterator(infix_const_reverse_iterator &&other) noexcept
        {
            if(!_path.empty()){
                clear_path(_path);
            }
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        infix_const_reverse_iterator &operator=(infix_const_reverse_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        infix_const_reverse_iterator &operator=(infix_const_reverse_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class postfix_iterator final
    {

    public:
        
        explicit postfix_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;

    public:
        
        bool operator==(postfix_iterator const &other) const noexcept;
        
        bool operator!=(postfix_iterator const &other) const noexcept;
        
        postfix_iterator &operator++();
        
        postfix_iterator const operator++(int not_used);
        
        iterator_data *operator*() const;

        ~postfix_iterator()
        {
            _tree->clear_path(_path);
        }

        postfix_iterator(postfix_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_iterator(postfix_iterator &&other) noexcept
        {
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_iterator &operator=(postfix_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        postfix_iterator &operator=(postfix_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class postfix_const_iterator final
    {

    public:
        
        explicit postfix_const_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;

    public:
        
        bool operator==(postfix_const_iterator const &other) const noexcept;
        
        bool operator!=(postfix_const_iterator const &other) const noexcept;
        
        postfix_const_iterator &operator++();
        
        postfix_const_iterator const operator++(int not_used);
        
        iterator_data const *operator*() const;
        
        ~postfix_const_iterator()
        {
            _tree->clear_path(_path);
        }

        postfix_const_iterator(postfix_const_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_const_iterator(postfix_const_iterator &&other) noexcept
        {
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_const_iterator &operator=(postfix_const_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        postfix_const_iterator &operator=(postfix_const_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class postfix_reverse_iterator final
    {

    public:
        
        explicit postfix_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;

    public:
        
        bool operator==(postfix_reverse_iterator const &other) const noexcept;
        
        bool operator!=(postfix_reverse_iterator const &other) const noexcept;
        
        postfix_reverse_iterator &operator++();
        
        postfix_reverse_iterator const operator++(int not_used);
        
        iterator_data *operator*() const;

        ~postfix_reverse_iterator()
        {
            _tree->clear_path(_path);
        }

        postfix_reverse_iterator(postfix_reverse_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_reverse_iterator(postfix_reverse_iterator &&other) noexcept
        {
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_reverse_iterator &operator=(postfix_reverse_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        postfix_reverse_iterator &operator=(postfix_reverse_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    class postfix_const_reverse_iterator final
    {

    public:
        
        explicit postfix_const_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree, binary_search_tree<tkey, tvalue>::node *subtree_root);
    
    private:
        const binary_search_tree<tkey, tvalue> *_tree;
        iterator_data *_ptr;
        std::queue<iterator_data*> _path;

    public:
        
        bool operator==(postfix_const_reverse_iterator const &other) const noexcept;
        
        bool operator!=(postfix_const_reverse_iterator const &other) const noexcept;
        
        postfix_const_reverse_iterator &operator++();
        
        postfix_const_reverse_iterator const operator++(int not_used);
        
        iterator_data const *operator*() const;
        
        ~postfix_const_reverse_iterator()
        {
            _tree->clear_path(_path);
        }

        postfix_const_reverse_iterator(postfix_const_reverse_iterator const &other){
            other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_const_reverse_iterator(postfix_const_reverse_iterator &&other) noexcept
        {
            if(!_path.empty()){
                _tree->clear_path(_path);
            }
            other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
        }

        postfix_const_reverse_iterator &operator=(postfix_const_reverse_iterator const &other){
            if(this != &other){
                other._tree->iterator_copy(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }

        postfix_const_reverse_iterator &operator=(postfix_const_reverse_iterator &&other) noexcept
        {
            if(this != &other){
                other._tree->iterator_move(_ptr, other._ptr, _tree, other._tree, _path, other._path);
            }
            return *this;
        }
    };
    
    // endregion iterators definition

protected:
    
    // region target operations strategies definition
    
    enum class insertion_of_existent_key_attempt_strategy
    {
        update_value,
        throw_an_exception
    };
    
    enum class disposal_of_nonexistent_key_attempt_strategy
    {
        do_nothing,
        throw_an_exception
    };
    
    // endregion target operations strategies definition
    
    // region target operations associated exception types
    
    class insertion_of_existent_key_attempt_exception final:
        public std::logic_error
    {
    
    private:
        
        tkey _key;
    
    public:
        
        explicit insertion_of_existent_key_attempt_exception(tkey const &key);
        
    public:
        
        tkey const &get_key() const noexcept;
    
    };
    
    class obtaining_of_nonexistent_key_attempt_exception final:
        public std::logic_error
    {
    
    private:
        
        tkey _key;
        
    public:
        
        explicit obtaining_of_nonexistent_key_attempt_exception(tkey const &key);
        
    public:
        
        tkey const &get_key() const noexcept;
        
    };
    
    class disposal_of_nonexistent_key_attempt_exception final:
        public std::logic_error
    {
    
    private:
        
        tkey _key;
    
    public:
        
        explicit disposal_of_nonexistent_key_attempt_exception(tkey const &key);
        
    public:
        
        tkey const &get_key() const noexcept;
    
    };
    
    // endregion target operations associated exception types
    
    // region template methods definition
    
    class template_method_basics:
        public logger_guardant
    {
    
    public:
    
        binary_search_tree<tkey, tvalue> *_tree;
        
    public:
    
        explicit template_method_basics(binary_search_tree<tkey, tvalue> *tree);
        
    protected:
        
        std::stack<node **> find_path(tkey const &key) const
        {
            std::stack<node **> result_path;

            node **path_finder = &(_tree->_root);
            auto const &comparer = _tree->_keys_comparer;

            while (true)
            {
                result_path.push(path_finder);

                if (*path_finder == nullptr)
                {
                    break;
                }

                auto comparison_result = comparer(key, (*path_finder)->key);
                if (!comparison_result)
                {
                    break;
                }
                if(comparison_result < 0){
                    path_finder = &((*path_finder)->left_subtree);
                }
                else{
                    path_finder = &((*path_finder)->right_subtree);
                }
            }

            return result_path;
        }

    protected:
    
        [[nodiscard]] logger *get_logger() const noexcept final;
        
    };

    class insertion_template_method:
        public template_method_basics,
        public allocator_guardant
    {
    
    public:

        binary_search_tree<tkey, tvalue> *_tree;

    private:

        binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy _insertion_strategy;
    
    public:
        
        explicit insertion_template_method(
            binary_search_tree<tkey, tvalue> *tree,
            typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy);
        
    public:
        
        void insert(tkey const &key, tvalue const &value);
        
        void insert(tkey const &key, tvalue &&value);
    
        void set_insertion_strategy(
            typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept;
    
    private:
        
        [[nodiscard]] allocator *get_allocator() const noexcept final;
        
    };
    
    class obtaining_template_method:
        public template_method_basics
    {
    
    public:

        binary_search_tree<tkey, tvalue> *_tree;

    public:
        
        explicit obtaining_template_method(binary_search_tree<tkey, tvalue> *tree);
    
    public:
        
        tvalue const &obtain(tkey const &key);

        std::vector<typename associative_container<tkey, tvalue>::key_value_pair> obtain_between(
            tkey const &lower_bound,
            tkey const &upper_bound,
            bool lower_bound_inclusive,
            bool upper_bound_inclusive)
        {
            std::vector<typename associative_container<tkey, tvalue>::key_value_pair> range;

            std::stack<node *> path;
            node *current = this->_tree->_root;
            while (true)
            {
                if (current == nullptr)
                {
                    break;
                }

                auto comparison_result = this->_tree->_keys_comparer(lower_bound, current->key);
                path.push(current);

                if (comparison_result == 0)
                {
                    if (lower_bound_inclusive)
                    {
                        break;
                    }

                    current = current->right_subtree;
                }
                else if (comparison_result < 0)
                {
                    current = current->left_subtree;
                }
                else
                {
                    current = current->right_subtree;
                }

                if (current == nullptr && this->_tree->_keys_comparer(path.top()->key, lower_bound) < (lower_bound_inclusive
                    ? 0
                    : 1))
                {
                    path = std::move(std::stack<node *>());
                }
            }

            auto it = infix_iterator(_tree, _tree->_root);
            while ((it != this->_tree->end_infix()) && (this->_tree->_keys_comparer(upper_bound, (*it)->get_key()) > (upper_bound_inclusive
                ? -1
                : 0)))
            {
                range.push_back(std::move(typename associative_container<tkey, tvalue>::key_value_pair((*it)->get_key(), (*it)->get_value())));
                ++it;
            }

            return range;
        }
    };
    
    class disposal_template_method:
        public template_method_basics,
        public allocator_guardant
    {

    public:

        binary_search_tree<tkey, tvalue> *_tree;

    private:

        binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy _disposal_strategy;
    
    public:
        
        explicit disposal_template_method(
            binary_search_tree<tkey, tvalue> *tree,
            typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy);
        
    public:
        
        virtual tvalue dispose(tkey const &key);
        
        void set_disposal_strategy(
            typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept;
    
    private:

        template<typename T>
        inline void swap(T &&one, T &&another)
        {
            T temp = std::move(one);
            one = std::move(another);
            another = std::move(temp);
        }
    
        [[nodiscard]] allocator *get_allocator() const noexcept final;
        
    };
    
    // endregion template methods definition

private:
    
    node *_root = nullptr;
    insertion_template_method *_insertion_template = nullptr;
    obtaining_template_method *_obtaining_template = nullptr;
    disposal_template_method *_disposal_template = nullptr;

protected:
    
    explicit binary_search_tree(
        typename binary_search_tree<tkey, tvalue>::insertion_template_method *insertion_template,
        typename binary_search_tree<tkey, tvalue>::obtaining_template_method *obtaining_template,
        typename binary_search_tree<tkey, tvalue>::disposal_template_method *disposal_template,
        std::function<int(tkey const &, tkey const &)>,
        allocator *allocator,
        logger *logger);

public:
    
    explicit binary_search_tree(
        std::function<int(tkey const &, tkey const &)> comparer = std::less<tkey>(),
        allocator *allocator = nullptr,
        logger *logger = nullptr,
        typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy = binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy::throw_an_exception,
        typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy = binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy::throw_an_exception);

public:
    
    binary_search_tree(binary_search_tree<tkey, tvalue> const &other);
    
    binary_search_tree(binary_search_tree<tkey, tvalue> &&other) noexcept;
    
    binary_search_tree<tkey, tvalue> &operator=(binary_search_tree<tkey, tvalue> const &other);
    
    binary_search_tree<tkey, tvalue> &operator=(binary_search_tree<tkey, tvalue> &&other) noexcept;
    
    ~binary_search_tree() override;

public:
    
    void insert(tkey const &key, tvalue const &value) final;
    
    void insert(tkey const &key, tvalue &&value) final;
    
    tvalue const &obtain(tkey const &key) final;
    
    std::vector<typename associative_container<tkey, tvalue>::key_value_pair> obtain_between(
        tkey const &lower_bound,
        tkey const &upper_bound,
        bool lower_bound_inclusive,
        bool upper_bound_inclusive) final;
    
    tvalue dispose(tkey const &key) final;
    
public:
    
    void set_insertion_strategy(
        typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept;
    
    void set_removal_strategy(
        typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept;

public:
    
    // region iterators requests definition
    
    prefix_iterator begin_prefix() const noexcept;
    
    prefix_iterator end_prefix() const noexcept;
    
    prefix_const_iterator cbegin_prefix() const noexcept;
    
    prefix_const_iterator cend_prefix() const noexcept;
    
    prefix_reverse_iterator rbegin_prefix() const noexcept;
    
    prefix_reverse_iterator rend_prefix() const noexcept;
    
    prefix_const_reverse_iterator crbegin_prefix() const noexcept;
    
    prefix_const_reverse_iterator crend_prefix() const noexcept;
    
    infix_iterator begin_infix() const noexcept;
    
    infix_iterator end_infix() const noexcept;
    
    infix_const_iterator cbegin_infix() const noexcept;
    
    infix_const_iterator cend_infix() const noexcept;
    
    infix_reverse_iterator rbegin_infix() const noexcept;
    
    infix_reverse_iterator rend_infix() const noexcept;
    
    infix_const_reverse_iterator crbegin_infix() const noexcept;
    
    infix_const_reverse_iterator crend_infix() const noexcept;
    
    postfix_iterator begin_postfix() const noexcept;
    
    postfix_iterator end_postfix() const noexcept;
    
    postfix_const_iterator cbegin_postfix() const noexcept;
    
    postfix_const_iterator cend_postfix() const noexcept;
    
    postfix_reverse_iterator rbegin_postfix() const noexcept;
    
    postfix_reverse_iterator rend_postfix() const noexcept;
    
    postfix_const_reverse_iterator crbegin_postfix() const noexcept;
    
    postfix_const_reverse_iterator crend_postfix() const noexcept;
    
    // endregion iterators requests definition

protected:
    
    // region subtree rotations definition
    
    void small_left_rotation(
        typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
        bool validate = true) const;
    
    void small_right_rotation(
        typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
        bool validate = true) const;
    
    void big_left_rotation(
        typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
        bool validate = true) const;
    
    void big_right_rotation(
        typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
        bool validate = true) const;
    
    void double_left_rotation(
        typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
        bool at_grandparent_first,
        bool validate = true) const;
    
    void double_right_rotation(
        typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
        bool at_grandparent_first,
        bool validate = true) const;
    
    // endregion subtree rotations definition
};

// region binary_search_tree<tkey, tvalue>::node methods implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::node::node(tkey const &key,tvalue const &value):
        key(key),
        value(value),
        left_subtree(nullptr),
        right_subtree(nullptr)
{
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::node::node(tkey const &key,tvalue &&value):
        key(key),
        value(std::move(value)),
        left_subtree(nullptr),
        right_subtree(nullptr)
{
}

// endregion binary_search_tree<tkey, tvalue>::node methods implementation

// region iterators implementation

// region iterator data implementation



// endregion iterator data implementation

// region prefix_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_iterator::prefix_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root):
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->prefix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::prefix_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::prefix_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator &binary_search_tree<tkey, tvalue>::prefix_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }
    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();
    
    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator const binary_search_tree<tkey, tvalue>::prefix_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::prefix_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *binary_search_tree<tkey, tvalue>::prefix_iterator::operator*() const
{
    return _ptr;
}

// endregion prefix_iterator implementation

// region prefix_const_iterator implementation

template<typename tkey, typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_const_iterator::prefix_const_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root) :
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->prefix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::prefix_const_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::prefix_const_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator &binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey, typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator const binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::prefix_const_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator*() const
{
    return _ptr;
}

// endregion prefix_const_iterator implementation

// region prefix_reverse_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::prefix_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root) :
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->prefix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator &binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator const binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator*() const
{
    return _ptr;
}

// endregion prefix_reverse_iterator implementation

// region prefix_const_reverse_iterator implementation

template<typename tkey, typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root) :
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->prefix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator &binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator const binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator*() const
{
    return _ptr;
}

// endregion prefix_const_reverse_iterator implementation

// region infix_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::infix_iterator::infix_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root):
        _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->infix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::infix_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::infix_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator &binary_search_tree<tkey, tvalue>::infix_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator const binary_search_tree<tkey, tvalue>::infix_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::infix_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<
    typename tkey,
    typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *binary_search_tree<tkey, tvalue>::infix_iterator::operator*() const
{
    return _ptr;
}

// endregion infix_iterator implementation

// region infix_const_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::infix_const_iterator::infix_const_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root) :
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    _tree->infix_path(0, _path, subtree_root);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::infix_const_iterator const &other) const noexcept
{
    return (_ptr->depth == other._ptr->depth) && (_ptr->key == other._ptr->key) && (_ptr->value == other._ptr->value);
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::infix_const_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator &binary_search_tree<tkey, tvalue>::infix_const_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator const binary_search_tree<tkey, tvalue>::infix_const_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::infix_const_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *binary_search_tree<tkey, tvalue>::infix_const_iterator::operator*() const
{
    return _ptr;
}

// endregion infix_const_iterator implementation

// region infix_reverse_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::infix_reverse_iterator::infix_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root):
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->infix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator &binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator const binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator*() const
{
    return _ptr;
}

// endregion infix_reverse_iterator implementation

// region infix_const_reverse_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::infix_const_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root):
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->infix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator &binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator const binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator*() const
{
    return _ptr;
}

// endregion infix_const_reverse_iterator implementation

// region postfix_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_iterator::postfix_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root) :
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->postfix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::postfix_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::postfix_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator &binary_search_tree<tkey, tvalue>::postfix_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator const binary_search_tree<tkey, tvalue>::postfix_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::postfix_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *binary_search_tree<tkey, tvalue>::postfix_iterator::operator*() const
{
    return _ptr;
}

// endregion postfix_iterator implementation

// region postfix_const_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_const_iterator::postfix_const_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root):
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->postfix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::postfix_const_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::postfix_const_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator &binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator const binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::postfix_const_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator*() const
{
    return _ptr;
}

// endregion postfix_const_iterator implementation

// region postfix_reverse_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::postfix_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root):
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->postfix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator &binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator const binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator*() const
{
    return _ptr;
}

// endregion postfix_reverse_iterator implementation

// region postfix_const_reverse_iterator implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(const binary_search_tree<tkey, tvalue> *tree,
    binary_search_tree<tkey, tvalue>::node *subtree_root) :
    _tree(tree)
{
    if(subtree_root == nullptr){
        _ptr = nullptr;
        return;
    }

    node *cur = subtree_root;
    _tree->postfix_path(0, _path, cur);
    _ptr = _path.front();
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator==(
    typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator const &other) const noexcept
{
    return _ptr == other._ptr;
}

template<typename tkey,typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator!=(
    typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator const &other) const noexcept
{
    return _ptr != other._ptr;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator &binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator++()
{
    if(_path.empty()){
        _tree->error_with_guard("Binary_search_tree: path is empty.\n");
    }

    _tree->clear_iterator_data(_ptr);
    _path.pop();
    _ptr = _path.front();

    return *this;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator const binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator++(
    int not_used)
{
    typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator tmp(*this);
    ++tmp;
    return tmp;
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator*() const
{
    return _ptr;
}

// endregion postfix_const_reverse_iterator implementation

// endregion iterators implementation

// region target operations associated exception types implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_exception::insertion_of_existent_key_attempt_exception(
    tkey const &key):
    std::logic_error("Attempt to insert already existing key inside the tree.\n"), _key(key)
{
}

template<typename tkey,typename tvalue>
tkey const &binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_exception::get_key() const noexcept
{
    return _key;
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::obtaining_of_nonexistent_key_attempt_exception::obtaining_of_nonexistent_key_attempt_exception(
    tkey const &key):
    std::logic_error("Attempt to obtain a value by non-existing key from the tree.\n"), _key(key)
{
}

template<typename tkey,typename tvalue>
tkey const &binary_search_tree<tkey, tvalue>::obtaining_of_nonexistent_key_attempt_exception::get_key() const noexcept
{
    return _key;
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_exception::disposal_of_nonexistent_key_attempt_exception(
    tkey const &key):
    std::logic_error("Attempt to dispose a value by non-existing key from the tree.\n"), _key(key)
{
}

template<typename tkey,typename tvalue>
tkey const &binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_exception::get_key() const noexcept
{
    return _key;
}

// endregion target operations associated exception types implementation

// region template methods implementation

// region binary_search_tree<tkey, tvalue>::template_method_basics implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::template_method_basics::template_method_basics(binary_search_tree<tkey, tvalue> *tree)
    : _tree(tree)
{
}

template<typename tkey,typename tvalue>
[[nodiscard]] inline logger *binary_search_tree<tkey, tvalue>::template_method_basics::get_logger() const noexcept
{
    return _tree->get_logger();
}

// endregion binary_search_tree<tkey, tvalue>::template_method_basics implementation

// region search_tree<tkey, tvalue>::insertion_template_method implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::insertion_template_method::insertion_template_method(binary_search_tree<tkey, tvalue> *tree,
    typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy):
    binary_search_tree<tkey, tvalue>::template_method_basics::template_method_basics(tree),
    _insertion_strategy(insertion_strategy),
    _tree(tree)
{
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::insertion_template_method::insert(tkey const &key,tvalue const &value)
{
    this->trace_with_guard("Binary_earch_tree: start insert.\n");

    node *free_space;
    node *cur = _tree->_root;

    if(cur == nullptr){
        try{
            free_space = reinterpret_cast<node*>(allocate_with_guard(sizeof(binary_search_tree::node), 1));
            allocator::construct(free_space, key, value);
        }
        catch(const std::exception &ex){
            this->error_with_guard("Binary_search_tree: failed to allocate.\n");
            throw ex;
        }
        cur = free_space;
        _tree->_root = cur;
        this->trace_with_guard("Binary_search_tree: end insert.\n");
        return;
    }

    while(cur != nullptr && cur->key != key){
        if(key < cur->key){
            if(cur->left_subtree != nullptr){
                cur = cur->left_subtree;
            }
            else{
                try{
                    free_space = reinterpret_cast<node*>(allocate_with_guard(sizeof(binary_search_tree::node), 1));
                    allocator::construct(free_space, key, value);
                }
                catch(const std::exception &ex){
                    this->error_with_guard("Binary_search_tree: failed to allocate.\n");
                    throw ex;
                }
                cur->left_subtree = free_space;
                this->trace_with_guard("Binary_search_tree: end insert.\n");
                return;
            }
        }
        else if(key > cur->key){
            if(cur->right_subtree != nullptr){
                cur = cur->right_subtree;
            }
            else{
                try{
                    free_space = reinterpret_cast<node*>(allocate_with_guard(sizeof(binary_search_tree::node), 1));
                    allocator::construct(free_space, key, value);
                }
                catch(const std::exception &ex){
                    this->error_with_guard("Binary_search_tree: failed to allocate.\n");
                    throw ex;
                }
                cur->right_subtree = free_space;
                this->trace_with_guard("Binary_search_tree: end insert.\n");
                return;
            }
        }
        else{
            this->trace_with_guard("Binary_search_tree: this node already exists.\n");
            switch(_insertion_strategy){
                case insertion_of_existent_key_attempt_strategy::throw_an_exception:
                    throw insertion_of_existent_key_attempt_exception(key);
                    break;
                case insertion_of_existent_key_attempt_strategy::update_value:
                    cur->value = value;
                    this->warning_with_guard("Binary_search_tree: value was updated.\n");
                    break;
            }
            return;
        }
    }
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::insertion_template_method::insert(tkey const &key,tvalue &&value)
{
    this->debug_with_guard("Binary_search_tree: start insert.\n");
    
    node *free_space;
    node *cur = _tree->_root;

    if(cur == nullptr){
        try{
            free_space = reinterpret_cast<node*>(allocate_with_guard(sizeof(binary_search_tree::node), 1));
            allocator::construct(free_space, key, value);
        }
        catch(const std::exception &ex){
            this->error_with_guard("Binary_search_tree: failed to allocate.\n");
            throw ex;
        }
        cur = free_space;
        _tree->_root = cur;
        this->trace_with_guard("Binary_search_tree: end insert.\n");
        return;
    }

    while(cur != nullptr){
        if(key < cur->key){
            if(cur->left_subtree != nullptr){
                cur = cur->left_subtree;
            }
            else{
                try{
                    free_space = reinterpret_cast<node*>(allocate_with_guard(sizeof(binary_search_tree::node), 1));
                    allocator::construct(free_space, key, value);
                }
                catch(const std::exception &ex){
                    this->error_with_guard("Binary_search_tree: failed to allocate.\n");
                    throw ex;
                }
                cur->left_subtree = free_space;
                this->trace_with_guard("Binary_search_tree: end insert.\n");
                return;
            }
        }
        else if(key > cur->key){
            if(cur->right_subtree != nullptr){
                cur = cur->right_subtree;
            }
            else{
                try{
                    free_space = reinterpret_cast<node*>(allocate_with_guard(sizeof(binary_search_tree::node), 1));
                    allocator::construct(free_space, key, value);
                }
                catch(const std::exception &ex){
                    this->error_with_guard("Binary_search_tree: failed to allocate.\n");
                    throw ex;
                }
                cur->right_subtree = free_space;
                this->trace_with_guard("Binary_search_tree: end insert.\n");
                return;
            }
        }
        else{
            this->trace_with_guard("Binary_search_tree: this node already exists.\n");
            switch(_insertion_strategy){
                case insertion_of_existent_key_attempt_strategy::throw_an_exception:
                    throw insertion_of_existent_key_attempt_exception(key);
                    break;
                case insertion_of_existent_key_attempt_strategy::update_value:
                    cur->value = value;
                    this->warning_with_guard("Binary_search_tree: value was updated.\n");
                    break;
            }
            return;
        }
    }
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::insertion_template_method::set_insertion_strategy(
    typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept
{
    _insertion_strategy = insertion_strategy;
    this->trace_with_guard("Binary_search_tree: new insertion strategy.\n");
}

template<typename tkey,typename tvalue>
allocator *binary_search_tree<tkey, tvalue>::insertion_template_method::get_allocator() const noexcept
{
    return this->_tree->get_allocator();
}

// endregion search_tree<tkey, tvalue>::insertion_template_method implementation

// region search_tree<tkey, tvalue>::obtaining_template_method implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::obtaining_template_method::obtaining_template_method(binary_search_tree<tkey, tvalue> *tree):
    binary_search_tree<tkey, tvalue>::template_method_basics::template_method_basics(tree),
    _tree(tree)
{
}

template<typename tkey,typename tvalue>
tvalue const &binary_search_tree<tkey, tvalue>::obtaining_template_method::obtain(tkey const &key)
{
    auto path = this->find_path(key);
    if (*(path.top()) == nullptr)
    {
        throw obtaining_of_nonexistent_key_attempt_exception(key);
    }

    tvalue const &got_value = (*(path.top()))->value;

    return got_value;
}

// endregion search_tree<tkey, tvalue>::obtaining_template_method implementation

// region search_tree<tkey, tvalue>::disposal_template_method implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::disposal_template_method::disposal_template_method(binary_search_tree<tkey, tvalue> *tree,
    typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy):
    binary_search_tree<tkey, tvalue>::template_method_basics::template_method_basics(tree), _tree(tree), _disposal_strategy(disposal_strategy)
{
}

template<typename tkey,typename tvalue>
tvalue binary_search_tree<tkey, tvalue>::disposal_template_method::dispose(tkey const &key)
{
    this->trace_with_guard("Binary_search_tree: start dispose.\n");
    auto path = this->find_path(key);
    if (*(path.top()) == nullptr)
    {
        switch (_disposal_strategy)
        {
            case disposal_of_nonexistent_key_attempt_strategy::throw_an_exception:
                throw disposal_of_nonexistent_key_attempt_exception(key);
            case disposal_of_nonexistent_key_attempt_strategy::do_nothing:
                return tvalue();
        }
    }

    bool flag = 0;

    if ((*(path.top()))->left_subtree != nullptr && (*(path.top()))->right_subtree != nullptr)
    {
        flag = 1;
        this->trace_with_guard("Binary_search_tree: removing node has subtrees.\n");
        
        auto *target_to_swap = *(path.top());
        auto **current = &((*(path.top()))->left_subtree);

        while (*current != nullptr)
        {
            path.push(current);
            current = &((*current)->right_subtree);
        }

        swap(std::move(target_to_swap->key), std::move((*(path.top()))->key));
        swap(std::move(target_to_swap->value), std::move((*(path.top()))->value));
    }

    tvalue value = std::move((*(path.top()))->value);
    node *leftover_subtree;

    if((*(path.top()))->left_subtree == nullptr && (*(path.top()))->right_subtree == nullptr){
        if(!flag){
            this->trace_with_guard("Binary_search_tree: removing node has no subtrees.\n");
        }
        leftover_subtree = nullptr;
    }
    else if ((*(path.top()))->left_subtree != nullptr){
        this->trace_with_guard("Binary_search_tree: removing node has left subtree.\n");
        leftover_subtree = (*(path.top()))->left_subtree;
    }
    else{
        this->trace_with_guard("Binary_search_tree: removing node has right subtrees.\n");
        leftover_subtree = (*(path.top()))->right_subtree;
    }

    allocator::destruct(*(path.top()));
    deallocate_with_guard(*(path.top()));

    *(path.top()) = leftover_subtree;
    this->trace_with_guard("Binary_search_tree: end dispose.\n");
    return value;
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::disposal_template_method::set_disposal_strategy(
    typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept
{
    _disposal_strategy = disposal_strategy;
    this->trace_with_guard("Binary_search_tree: new disposal strategy.\n");
}

template<typename tkey,typename tvalue>
[[nodiscard]] inline allocator *binary_search_tree<tkey, tvalue>::disposal_template_method::get_allocator() const noexcept
{
    return this->_tree->get_allocator();
}

// endregion search_tree<tkey, tvalue>::disposal_template_method implementation

// endregion template methods

// region construction, assignment, destruction implementation

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(
    typename binary_search_tree<tkey, tvalue>::insertion_template_method *insertion_template,
    typename binary_search_tree<tkey, tvalue>::obtaining_template_method *obtaining_template,
    typename binary_search_tree<tkey, tvalue>::disposal_template_method *disposal_template,
    std::function<int(tkey const &, tkey const &)> comparer, allocator *allocator, logger *logger):
    search_tree<tkey, tvalue>(comparer, logger, allocator),
    _root(nullptr),
    _insertion_template(insertion_template),
    _obtaining_template(obtaining_template),
    _disposal_template(disposal_template)
{
    this->debug_with_guard("Binary_search_tree: constructor.\n");
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(
    std::function<int(tkey const &, tkey const &)> keys_comparer, allocator *allocator, logger *logger,
    typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy,
    typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy):
    binary_search_tree(
        new binary_search_tree<tkey, tvalue>::insertion_template_method(this, insertion_strategy),
        new binary_search_tree<tkey, tvalue>::obtaining_template_method(this),
        new binary_search_tree<tkey, tvalue>::disposal_template_method(this, disposal_strategy),
        keys_comparer,
        allocator,
        logger)
{
    this->debug_with_guard("Binary_search_tree: constructor.\n");
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(binary_search_tree<tkey, tvalue> const &other)
{
    this->debug_with_guard("Binary_search_tree: start copy constructor.\n");
    if(_root){
        clear(_root);
    }
    _root = copy(other._root);

    if(_insertion_template != nullptr){
        delete _insertion_template;
    }
    _insertion_template = new binary_search_tree<tkey, tvalue>::insertion_template_method(*other._insertion_template);

    if(_disposal_template != nullptr){
        delete _disposal_template;
    }
    _disposal_template = new binary_search_tree<tkey, tvalue>::disposal_template_method(*other._disposal_template);

    if(_obtaining_template != nullptr){
        delete _obtaining_template;
    }
    _obtaining_template = new binary_search_tree<tkey, tvalue>::obtaining_template_method(*other._obtaining_template);
    
    this->debug_with_guard("Binary_search_tree: end copy constructor.\n");
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(binary_search_tree<tkey, tvalue> &&other) noexcept
{
    this->debug_with_guard("Binary_search_tree: start move constructor.\n");

    if(_root){
        clear(_root);
    }

    _root = std::exchange(other._root, nullptr);

    if(_insertion_template != nullptr){
        delete _insertion_template;
    }
    _insertion_template = new binary_search_tree<tkey, tvalue>::insertion_template_method(*other._insertion_template);

    if(_disposal_template != nullptr){
        delete _disposal_template;
    }
    _disposal_template = new binary_search_tree<tkey, tvalue>::disposal_template_method(*other._disposal_template);

    if(_obtaining_template != nullptr){
        delete _obtaining_template;
    }
    _obtaining_template = new binary_search_tree<tkey, tvalue>::obtaining_template_method(*other._obtaining_template);

    this->debug_with_guard("Binary_search_tree: end move constructor.\n");
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue> &binary_search_tree<tkey, tvalue>::operator=(binary_search_tree<tkey, tvalue> const &other)
{
    this->debug_with_guard("Binary_search_tree: start copy operator.\n");
    if (this != &other)
    {
        if(_root != nullptr){
            clear(_root);
        }
        _root = copy(other._root);

        if(_insertion_template != nullptr){
            delete _insertion_template;
        }
        _insertion_template = new binary_search_tree<tkey, tvalue>::insertion_template_method(*other._insertion_template);

        if(_disposal_template != nullptr){
            delete _disposal_template;
        }
        _disposal_template = new binary_search_tree<tkey, tvalue>::disposal_template_method(*other._disposal_template);

        if(_obtaining_template != nullptr){
            delete _obtaining_template;
        }
        _obtaining_template = new binary_search_tree<tkey, tvalue>::obtaining_template_method(*other._obtaining_template);
    }
    this->debug_with_guard("Binary_search_tree: end copy operator.\n");
    return *this;
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue> &binary_search_tree<tkey, tvalue>::operator=(binary_search_tree<tkey, tvalue> &&other) noexcept
{
    this->debug_with_guard("Binary_search_tree: start move operator.\n");
    if (this != &other)
    {
        if(_root != nullptr){
            clear(_root);
        }
        _root = std::exchange(other._root, nullptr);
        if(_insertion_template != nullptr){
            delete _insertion_template;
        }
        _insertion_template = new binary_search_tree<tkey, tvalue>::insertion_template_method(*other._insertion_template);

        if(_disposal_template != nullptr){
            delete _disposal_template;
        }
        _disposal_template = new binary_search_tree<tkey, tvalue>::disposal_template_method(*other._disposal_template);

        if(_obtaining_template != nullptr){
            delete _obtaining_template;
        }
        _obtaining_template = new binary_search_tree<tkey, tvalue>::obtaining_template_method(*other._obtaining_template);
    }
    this->debug_with_guard("Binary_search_tree: end move operator.\n");
    return *this;
}

template<typename tkey,typename tvalue>
binary_search_tree<tkey, tvalue>::~binary_search_tree()
{
    this->debug_with_guard("Binary_search_tree: start destructor.\n");
    clear(_root);
    if (_insertion_template != nullptr){
        delete _insertion_template;
    }
    if (_obtaining_template != nullptr){
        delete _obtaining_template;
    } 
    if (_disposal_template != nullptr){
        delete _disposal_template;
    } 
    this->debug_with_guard("Binary_search_tree: end destructor.\n");
}

// endregion construction, assignment, destruction implementation

// region associative_container<tkey, tvalue> contract implementation

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::insert(tkey const &key,tvalue const &value)
{
    this->debug_with_guard("Binary_search_tree: start insert.\n");
    _insertion_template->insert(key, value);
    this->debug_with_guard("Binary_search_tree: end insert.\n");
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::insert(tkey const &key, tvalue &&value)
{
    this->debug_with_guard("Binary_search_tree: start move insert.\n");
    _insertion_template->insert(key, std::move(value));
    this->debug_with_guard("Binary_search_tree: end move insert.\n");
}

template<typename tkey,typename tvalue>
tvalue const &binary_search_tree<tkey, tvalue>::obtain(tkey const &key)
{
    this->debug_with_guard("Binary_search_tree: obtain.\n");
    return _obtaining_template->obtain(key);
}

template<typename tkey, typename tvalue>
std::vector<typename associative_container<tkey, tvalue>::key_value_pair> binary_search_tree<tkey, tvalue>::obtain_between(
    tkey const &lower_bound,
    tkey const &upper_bound,
    bool lower_bound_inclusive,
    bool upper_bound_inclusive)
{
    this->debug_with_guard("Binary_search_tree: obtain between.\n");
    return _obtaining_template->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
}

template<typename tkey,typename tvalue>
tvalue binary_search_tree<tkey, tvalue>::dispose(tkey const &key)
{
    this->debug_with_guard("Binary_search_tree: dispose.\n");
    return _disposal_template->dispose(key);
}

// endregion associative_containers contract implementations

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::set_insertion_strategy(
    typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept
{
    this->trace_with_guard("Binary_search_tree: start setting insertion strategy.\n");
    _insertion_template->set_insertion_strategy(insertion_strategy);
    this->trace_with_guard("Binary_search_tree: end setting insertion strategy.\n");
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::set_removal_strategy(
    typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept
{
    this->trace_with_guard("Binary_search_tree: start setting removal strategy.\n");
    _disposal_template->set_disposal_strategy(disposal_strategy);
    this->trace_with_guard("Binary_search_tree: end setting removal strategy.\n");
}

// region iterators requesting implementation

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator binary_search_tree<tkey, tvalue>::begin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator binary_search_tree<tkey, tvalue>::end_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator binary_search_tree<tkey, tvalue>::cbegin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator binary_search_tree<tkey, tvalue>::cend_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator binary_search_tree<tkey, tvalue>::rbegin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_reverse_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator binary_search_tree<tkey, tvalue>::rend_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_reverse_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator binary_search_tree<tkey, tvalue>::crbegin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator binary_search_tree<tkey, tvalue>::crend_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator binary_search_tree<tkey, tvalue>::begin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey, typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator binary_search_tree<tkey, tvalue>::end_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator binary_search_tree<tkey, tvalue>::cbegin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator binary_search_tree<tkey, tvalue>::cend_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator binary_search_tree<tkey, tvalue>::rbegin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_reverse_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator binary_search_tree<tkey, tvalue>::rend_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_reverse_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator binary_search_tree<tkey, tvalue>::crbegin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator binary_search_tree<tkey, tvalue>::crend_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator binary_search_tree<tkey, tvalue>::begin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator binary_search_tree<tkey, tvalue>::end_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator binary_search_tree<tkey, tvalue>::cbegin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator binary_search_tree<tkey, tvalue>::cend_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator binary_search_tree<tkey, tvalue>::rbegin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_reverse_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator binary_search_tree<tkey, tvalue>::rend_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_reverse_iterator(this, nullptr);
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator binary_search_tree<tkey, tvalue>::crbegin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator(this, dynamic_cast<typename binary_search_tree<tkey, tvalue>::node *>(_root));
}

template<typename tkey,typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator binary_search_tree<tkey, tvalue>::crend_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator(this, nullptr);
}

// endregion iterators request implementation

// region subtree rotations implementation

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::small_left_rotation(
    binary_search_tree<tkey, tvalue>::node *&subtree_root,bool validate) const
{
    this->debug_with_guard("Binary_search_tree: start small left rotation.\n");

    if(validate && subtree_root == nullptr|| subtree_root->right_subtree == nullptr){
        this->error_with_guard("Binary_search_tree: failed to rotate.\n");
        throw std::logic_error("Binary_search_tree: failed to rotate.\n");
    }

    node *new_root = subtree_root->right_subtree;
    subtree_root->right_subtree = new_root->left_subtree;
    subtree_root->left_subtree = subtree_root;
    subtree_root = new_root;

    this->debug_with_guard("Binary_search_tree: end small left rotation.\n");
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::small_right_rotation(
    binary_search_tree<tkey, tvalue>::node *&subtree_root,bool validate) const
{
    this->debug_with_guard("Binary_search_tree: start small right rotation.\n");

    if(validate && subtree_root == nullptr|| subtree_root->left_subtree == nullptr){
        this->error_with_guard("Binary_search_tree: failed to rotate.\n");
        throw std::logic_error("Binary_search_tree: failed to rotate.\n");
    }

    node *new_root = subtree_root->left_subtree;
    subtree_root->left_subtree = new_root->right_subtree;
    subtree_root->right_subtree = subtree_root;
    subtree_root = new_root;

    if(validate && !validation(subtree_root)){
        this->error_with_guard("Binary_search_tree: failed after rotation.\n");
        throw std::logic_error("Binary_search_tree: failed after rotation.\n");
    }

    this->debug_with_guard("Binary_search_tree: end small right rotation.\n");
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::big_left_rotation(
    binary_search_tree<tkey, tvalue>::node *&subtree_root,bool validate) const
{
    this->debug_with_guard("Binary_search_tree: start big left rotation.\n");
    small_right_rotation(subtree_root->right_subtree, validate);
    small_left_rotation(subtree_root, validate);
    this->debug_with_guard("Binary_search_tree: end big left rotation.\n");
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::big_right_rotation(
    binary_search_tree<tkey, tvalue>::node *&subtree_root,bool validate) const
{
    this->debug_with_guard("Binary_search_tree: start big right rotation.\n");
    small_left_rotation(subtree_root->left_subtree, validate);
    small_right_rotation(subtree_root, validate);
    this->debug_with_guard("Binary_search_tree: end big right rotation.\n");
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::double_left_rotation(
    binary_search_tree<tkey, tvalue>::node *&subtree_root, bool at_grandparent_first, bool validate) const
{
    this->debug_with_guard("Binary_search_tree: start double left rotation.\n");
    if(at_grandparent_first){
        small_left_rotation(subtree_root, validate);
    }
    else{
        small_left_rotation(subtree_root->right_subtree, validate);
    }
    small_left_rotation(subtree_root, validate);
    this->debug_with_guard("Binary_search_tree: end double left rotation.\n");
}

template<typename tkey,typename tvalue>
void binary_search_tree<tkey, tvalue>::double_right_rotation(
    binary_search_tree<tkey, tvalue>::node *&subtree_root,bool at_grandparent_first,bool validate) const
{
    this->debug_with_guard("Binary_search_tree: start double right rotation.\n");
    if(at_grandparent_first){
        small_right_rotation(subtree_root, validate);
    }
    else{
        small_right_rotation(subtree_root->right_subtree, validate);
    }
    small_right_rotation(subtree_root, validate);
    this->debug_with_guard("Binary_search_tree: end double right rotation.\n");
}

// endregion subtree rotations implementation

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H