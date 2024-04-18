#include <not_implemented.h>

#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    debug_with_guard("Allocator_sorted_list: start destructor.\n");
    logger *_logger = get_logger();
    allocator::destruct(&get_mutex());
    deallocate_with_guard(_trusted_memory);
    if(_logger){
        _logger->debug("Allocator_sorted_list: end destructor.\n");
    }
}

allocator_sorted_list::allocator_sorted_list(allocator_sorted_list &&other) noexcept
{
    logger *_logger = other.get_logger();
    if(_logger){
        _logger->debug("Allocator_sorted_list: start move constructor.\n");
    }

    if(_trusted_memory){
        deallocate_with_guard(_trusted_memory);
    }
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if(_logger){
        _logger->debug("Allocator_sorted_list: end move constructor.\n");
    }
}

allocator_sorted_list &allocator_sorted_list::operator=(allocator_sorted_list &&other) noexcept
{
    logger *_logger = other.get_logger();
    if(_logger){
        _logger->debug("Allocator_sorted_list: start move operator.\n");
    }

    if(this != &other){
        if(_trusted_memory){
            deallocate_with_guard(_trusted_memory);
        }
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }

    if(_logger){
        _logger->debug("Allocator_sorted_list: end move operator.\n");
    }
    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if(logger){
        logger->debug("Allocator_sorted_list: start constructor.\n");
    }
    auto meta = sizeof(size_t) + sizeof(allocator*) + sizeof(class logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex);
    auto meta_block = sizeof(size_t) + sizeof(void*);
    
    if(space_size < meta_block + sizeof(void*)){
        if(logger){
            logger->error("Allocator_sorted_list: failed to construct.\n");
        }
        throw std::logic_error("Allocator_sorted_list: failed to construct.\n");
    }
    auto total = space_size + meta;

    try{
        if(parent_allocator){
            _trusted_memory = parent_allocator->allocate(total, 1);
        }
        else{
            _trusted_memory = ::operator new(total);
        }
    }
    catch(std::bad_alloc const &ex){
        if(logger){
            logger->error("Allocator_sorted_list: failed to construct.\n");
        }
        throw ex;
    }

    unsigned char *result = reinterpret_cast<unsigned char*>(_trusted_memory);

    *reinterpret_cast<allocator**>(result) = parent_allocator;
    result += sizeof(allocator*);

    *reinterpret_cast<class logger**>(result) = logger;
    result += sizeof(class logger*);

    *reinterpret_cast<size_t*>(result) = space_size;
    result += sizeof(size_t);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(result) = allocate_fit_mode;
    result += sizeof(allocator_with_fit_mode::fit_mode);

    *reinterpret_cast<void**>(result) = result + sizeof(void*) + sizeof(std::mutex);
    result += sizeof(void*);

    allocator::construct(reinterpret_cast<std::mutex*>(result));
    result += sizeof(std::mutex);

    *reinterpret_cast<void**>(result) = nullptr;
    result += sizeof(void*);
    *reinterpret_cast<size_t*>(result) = space_size;

    if(logger){
        logger->debug("Allocator_sorted_list: end constructor.\n");
    }
}

[[nodiscard]] void *allocator_sorted_list::allocate(size_t value_size, size_t values_count)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard("Allocator_sorted_list: start allocate.\n");

    auto size = value_size*values_count;
    if(size < sizeof(void*)){
        size = sizeof(void*);
        warning_with_guard("Allocator_sorted_list: requested size was changed.\n");
    }

    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

    size_t avail = 0;

    void *block = nullptr;
    size_t prev_size = 0;
    void *prev = nullptr;
    void *next = nullptr;

    void *cur = get_first_free();
    void *last = nullptr;
    
    while(cur){
        size_t cur_size = get_free_size(cur);
        avail += cur_size;
        if(!cur_size){
            break;
        }
        if(cur_size >= size && (!block || better_fit(cur_size, prev_size, fit_mode))){
            block = cur;
            prev = last;
            next = *reinterpret_cast<void**>(cur);
            prev_size = cur_size;
        }
        last = cur;
        cur = *reinterpret_cast<void**>(cur);
    }

    if(!block){
        error_with_guard("Allocator_sorted_list: failed to allocate.\n");
        throw std::bad_alloc();
    }

    auto meta = sizeof(size_t) + sizeof(allocator*);
    auto result = meta + size;

    auto diff = get_free_size(block) - size;
    if(diff > 0 && diff < meta){
        warning_with_guard("Allocator_sorted_list: requested size was changed.\n");
        size += diff;
        result = size + meta;
    }

    else if(diff > 0){
        void **next_2 = reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(block) + result);
        *reinterpret_cast<size_t*>(next_2 + 1) = diff - sizeof(void*) - sizeof(size_t);
        if(next_2 + diff != next){
            *next_2 = next;
        }
        else{
            merge(next_2, next, 0);
        }
        if(prev){
            void **next_block = reinterpret_cast<void**>(prev);
            *next_block = next_2;
        }
        else{
            set_first_block(next_2);
        }
    }

    void **prev_block = reinterpret_cast<void**>(block);
    prev_block = nullptr;
    size_t *prev_size_ptr = reinterpret_cast<size_t*>(prev_block + 1);
    prev_size_ptr = nullptr;

    size_t *block_size = reinterpret_cast<size_t*>(block);
    *block_size = size;

    *reinterpret_cast<allocator**>(block_size + 1) = this;

    void *total = reinterpret_cast<unsigned char*>(block) + meta;

    std::vector<allocator_test_utils::block_info> b_info = get_blocks_info();
    print_info(b_info);

    information_with_guard("Allocator_sorted_list: available memory = " + std::to_string(avail));

    debug_with_guard("Allocator_sorted_list: end allocate.\n");
    return total;
}

bool allocator_sorted_list::better_fit(size_t cur_size, size_t prev_size, allocator_with_fit_mode::fit_mode mode)
{
    switch(mode){
        case allocator_with_fit_mode::fit_mode::first_fit:
            return true;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            return cur_size < prev_size || !prev_size;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            return cur_size > prev_size;
        default:
            return false;
    }
}

void allocator_sorted_list::deallocate(void *at)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    debug_with_guard("Allocator_sorted_list: start deallocate.\n");

    std::string block_array = get_block_info(at);
    debug_with_guard("Allocator_sorted_list: deallocate " + block_array + ".\n");

    size_t meta = sizeof(allocator*) + sizeof(size_t);
    size_t avail = 0;
    void *block = reinterpret_cast<unsigned char*>(at) - meta;
    size_t block_size = *reinterpret_cast<allocator::block_size_t*>(block);
    if(get_block_allocator(block) != this){
        error_with_guard("Allocator_sorted_list: wrong allocator.\n");
        throw std::logic_error("Allocator_sorted_list: wrong allocator.\n");
    }

    void *cur_avail = get_first_free();
    void *prev_avail = nullptr;
    void *next_avail = nullptr;

    while(cur_avail){
        avail += get_free_size(cur_avail);
        void *cur_occ;
        if((!prev_avail && cur_avail != get_first_block()) || cur_avail == get_first_block()){
            cur_occ = get_first_block();
        }
        else{
            cur_occ = reinterpret_cast<unsigned char*>(prev_avail) + get_free_size(prev_avail) + sizeof(void*) + sizeof(size_t);
        }
        
        while(cur_occ != cur_avail){
            size_t size = *reinterpret_cast<allocator::block_size_t*>(cur_occ);
            if(cur_occ == block){
                break;
            }
            cur_occ = reinterpret_cast<unsigned char*>(cur_occ) + sizeof(void*) + sizeof(size_t) + size;
        }
        if(cur_occ == block){
            break;
        }
        prev_avail = cur_avail;
        cur_avail = *reinterpret_cast<void**>(cur_avail);
    }

    if(block == get_first_block()){
        if(reinterpret_cast<unsigned char*>(block) + block_size + meta == cur_avail){
            merge(block, cur_avail, 1);
        }
        else{
            size_t *old_size = reinterpret_cast<size_t*>(block);
            old_size = nullptr;
            void **new_ptr = reinterpret_cast<void**>(block);
            *new_ptr = cur_avail;
            *reinterpret_cast<size_t*>(new_ptr + 1) = block_size;
        }
        set_first_block(block);

        std::vector<allocator_test_utils::block_info> blocks_info = get_blocks_info();
        print_info(blocks_info);

        debug_with_guard("Allocator_sorted_list: end deallocate.\n");
        return;
    }
    
    if(cur_avail == reinterpret_cast<unsigned char*>(block) + sizeof(size_t) + sizeof(allocator*) + block_size && cur_avail){
        merge(block, cur_avail, 1);
        if(prev_avail){
            *reinterpret_cast<void**>(prev_avail) = block;
            if(reinterpret_cast<unsigned char*>(prev_avail) + sizeof(size_t) + sizeof(void*) + get_free_size(prev_avail) == block){
                merge(prev_avail, block, 0);
            }
        }
        else{
            set_first_block(block);
        }
    }
    else if(cur_avail == reinterpret_cast<unsigned char*>(block) + sizeof(size_t) + sizeof(allocator*) + block_size && !cur_avail){
        size_t *old_size = reinterpret_cast<size_t*>(block);
        old_size = nullptr;
        void **new_ptr = reinterpret_cast<void**>(block);
        *new_ptr = cur_avail;
        *reinterpret_cast<size_t*>(new_ptr + 1) = block_size;

        *reinterpret_cast<void**>(prev_avail) = block;
        if(reinterpret_cast<unsigned char*>(prev_avail) + sizeof(size_t) + sizeof(void*) + get_free_size(prev_avail) == block){
            merge(prev_avail, block, 0);
        }
    }
    else{
        size_t *old_size = reinterpret_cast<size_t*>(block);
        old_size = nullptr;
        void **new_ptr = reinterpret_cast<void**>(block);
        *new_ptr = cur_avail;
        *reinterpret_cast<size_t*>(new_ptr + 1) = block_size;

        if(prev_avail){
            *reinterpret_cast<void**>(prev_avail) = block;
            if(reinterpret_cast<unsigned char*>(prev_avail) + sizeof(size_t) + sizeof(void*) + get_free_size(prev_avail) == block){
                merge(prev_avail, block, 0);
            }
        }
        else{
            set_first_block(block);
        }
    }

    std::vector<allocator_test_utils::block_info> blocks_info = get_blocks_info();
    print_info(blocks_info);
    information_with_guard("Allocator_sorted_list: free memory - " + std::to_string(avail) + ".\n");
    debug_with_guard("Allocator_sorted_list: end deallocate.\n");
}

inline void allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*)) = mode;
}

inline allocator_with_fit_mode::fit_mode allocator_sorted_list::get_fit_mode()
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t));
}

inline allocator *allocator_sorted_list::get_allocator() const
{
    return *reinterpret_cast<allocator**>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    void *cur_avail = get_first_free();
    void *prev_avail = nullptr;
    void *cur_occ = nullptr;
    void *prev_occ = nullptr;

    std::vector<allocator_test_utils::block_info> blocks_info;

    while(cur_avail){
        if(!prev_avail){
            cur_occ = get_first_block();
        }
        else{
            cur_occ = reinterpret_cast<unsigned char*>(prev_avail) + sizeof(void*) + sizeof(size_t) + get_free_size(prev_avail);
        }

        while(cur_occ != cur_avail){
            size_t occ_size = *reinterpret_cast<allocator::block_size_t*>(cur_occ);
            allocator_test_utils::block_info occ_block;
            occ_block.block_size = occ_size;
            occ_block.is_block_occupied = true;
            blocks_info.push_back(occ_block);

            prev_occ = cur_occ;
            cur_occ = reinterpret_cast<unsigned char*>(cur_occ) + sizeof(size_t) + sizeof(allocator*) + occ_size;
        }
        allocator_test_utils::block_info avail_block;
        avail_block.block_size = get_free_size(cur_avail);
        avail_block.is_block_occupied = false;
        blocks_info.push_back(avail_block);

        prev_avail = cur_avail;
        cur_avail = *reinterpret_cast<void**>(cur_avail);
    }
    return blocks_info;
}

inline logger *allocator_sorted_list::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<allocator**>(_trusted_memory) + 1);
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "Allocator_sorted_list";
}

std::string allocator_sorted_list::get_block_info(void *block) const noexcept
{
    unsigned char *bytes = reinterpret_cast<unsigned char*>(block);
    size_t size = *reinterpret_cast<allocator::block_size_t*>(bytes - sizeof(size_t) - sizeof(allocator*));
    std::string bytes_arr = "";
    for(block_size_t i = 0; i < size; i++){
        bytes_arr += std::to_string(*bytes);
        bytes += sizeof(unsigned char);
        bytes_arr += ' ';
    }
    return bytes_arr;
}

void allocator_sorted_list::print_info(std::vector<allocator_test_utils::block_info> blocks_info) const noexcept
{
    std::string info = "blocks info: ";
    for(auto block : blocks_info){
        if(block.is_block_occupied){
            info += "occupied, ";
        }
        else{
            info += "free, ";
        }
        info += "size = " + std::to_string(block.block_size) += ";";
    }
    debug_with_guard(info);
}

void *allocator_sorted_list::get_first_free() const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode));
}

allocator::block_size_t allocator_sorted_list::get_free_size(void *block) const noexcept
{
    return *reinterpret_cast<allocator::block_size_t*>(reinterpret_cast<void**>(block) + 1);
}

void allocator_sorted_list::set_first_block(void *block) const noexcept
{
    void **first_block = reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode));
    *first_block = block;
}

void *allocator_sorted_list::get_first_block() const noexcept
{
    return reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void*) + sizeof(std::mutex);
}

void allocator_sorted_list::merge (void *first, void *second, int type) noexcept
{
    void *next = *reinterpret_cast<void**>(second);
    size_t second_size = get_free_size(second);
    size_t first_size;
    if(!type){
        first_size = get_free_size(first);
    }
    else{
        first_size = *reinterpret_cast<allocator::block_size_t*>(first);
        size_t *old_size = reinterpret_cast<size_t*>(first);
        old_size = nullptr;
        allocator **all = reinterpret_cast<allocator**>(old_size + 1);
        all = nullptr;
    }
    void **new_ptr = reinterpret_cast<void**>(first);
    *new_ptr = next;
    *reinterpret_cast<size_t*>(new_ptr + 1) = first_size +second_size + sizeof(void*) + sizeof(size_t);
    clear_block(second);
}

void allocator_sorted_list::clear_block(void *block) const noexcept
{
    void **ptr = reinterpret_cast<void**>(block);
    size_t *size = reinterpret_cast<size_t*>(ptr + 1);
    ptr = nullptr;
    size = nullptr;
}

allocator *allocator_sorted_list::get_block_allocator(void *block) const noexcept
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char*>(block) + sizeof(size_t));
}

std::mutex &allocator_sorted_list::get_mutex() const noexcept
{
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void*));
}