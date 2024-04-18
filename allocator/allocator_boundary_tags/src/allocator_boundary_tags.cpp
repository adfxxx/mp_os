#include <not_implemented.h>

#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    debug_with_guard("Allocator_boundary_tags: start destructor.\n");
    logger *_logger = get_logger();
    allocator::destruct(&get_mutex());
    deallocate_with_guard(_trusted_memory);
    if(_logger){
        _logger->debug("Allocator_boundary_tags: end destructor.\n");
    }
}

allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags &&other) noexcept
{
    logger *_logger = get_logger();
    if(_logger){
        _logger->debug("Allocator_boundary_tags: start move constructor.\n");
    }

    if(_trusted_memory){
        deallocate_with_guard(_trusted_memory);
    }
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if(_logger){
        _logger->debug("Allocator_boundary_tags: end move constructor.\n");
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(allocator_boundary_tags &&other) noexcept
{
    logger *_logger = get_logger();
    if(_logger){
        _logger->debug("Allocator_boundary_tags: start move operator.\n");
    }

    if(this != &other){
        if(_trusted_memory){
            deallocate_with_guard(_trusted_memory);
        }
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }

    if(_logger){
        _logger->debug("Allocator_boundary_tags: end move operator.\n");
    }
    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if(logger){
        logger->debug("Allocator_boundary_tags: start constructor.\n");
    }
    //p_all, log, fit, ptr, mutex, trusted_memory
    size_t meta = sizeof(allocator*) + sizeof(class logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void*) + sizeof(std::mutex)+ sizeof(size_t);
    //all, ptr prev, ptr next, size
    size_t meta_block = sizeof(allocator*) + 2*sizeof(void*) + sizeof(size_t);
    
    if(space_size < meta_block + sizeof(void*)){
        if(logger){
            logger->error("Allocator_boundary_tags: failed to construct.\n");
        }
        throw std::logic_error("Allocator_boundary_tags: failed to construct.\n");
    }

    size_t total = space_size + meta;

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
            logger->error("Allocator_boundary_tags: failed to construct.\n");
        }
        throw ex;
    }

    unsigned char *result = reinterpret_cast<unsigned char*>(_trusted_memory);

    *reinterpret_cast<size_t*>(result) = space_size;
    result += sizeof(size_t);

    *reinterpret_cast<allocator**>(result) = parent_allocator;
    result += sizeof(allocator*);

    *reinterpret_cast<class logger**>(result) = logger;
    result += sizeof(class logger*);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(result) = allocate_fit_mode;
    result += sizeof(allocator_with_fit_mode::fit_mode);

    allocator::construct(reinterpret_cast<std::mutex*>(result));
    result += sizeof(std::mutex);

    *reinterpret_cast<void**>(result) = nullptr;
    result += sizeof(void*);

    if(logger){
        logger->debug("Allocator_boundary_tags: end constructor.\n");
    }
}

[[nodiscard]] void *allocator_boundary_tags::allocate(size_t value_size, size_t values_count)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard("Allocator_boundary_tags: start allocate.\n");

    auto size = value_size*values_count;
    if(size < sizeof(void*)){
        size = sizeof(void*);
        warning_with_guard("Allocator_boundary_tags: requested size was changed.\n");
    }

    size_t meta_block = 2*sizeof(void*) + sizeof(size_t) + sizeof(allocator*);
    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

    void *prev_occ = nullptr;
    size_t prev_size = 0;

    void *cur = nullptr;
    void *cur_prev = nullptr;
    void *cur_next = nullptr;

    void *cur_occ = get_first_occ();

    size_t free_size = *reinterpret_cast<size_t*>(_trusted_memory);

    if(!cur_occ && free_size >= size + meta_block){
        cur = get_first_block();
    }

    while(cur_occ){
        void *cur_free;
        if(!prev_occ){
            cur_free = get_first_block();
        }
        else{
            cur_free = reinterpret_cast<unsigned char*>(prev_occ) + meta_block + *reinterpret_cast<size_t*>(prev_occ);
        }

        if(cur_free != cur_occ){
            free_size = reinterpret_cast<unsigned char*>(cur_occ) - reinterpret_cast<unsigned char*>(cur_free);
            if(free_size >= size + meta_block){
                if(fit_mode == allocator_with_fit_mode::fit_mode::first_fit || 
                (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit && free_size < prev_size || prev_size == 0) ||
                (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit && free_size > prev_size)){
                    cur = cur_free;
                    prev_size = free_size;
                    cur_prev = prev_occ;
                    cur_next = cur_occ;
                }
            }
        }
        prev_occ = cur_occ;
        cur_occ = get_next(cur_occ);
    }

    if(!cur_occ && prev_occ && prev_occ != get_end()){
        void *cur_free = reinterpret_cast<unsigned char*>(prev_occ) + meta_block + *reinterpret_cast<size_t*>(prev_occ);
        free_size = reinterpret_cast<unsigned char*>(get_end()) - reinterpret_cast<unsigned char*>(cur_free);

        if(free_size >= size + meta_block){
            if(fit_mode == allocator_with_fit_mode::fit_mode::first_fit && !cur){
                cur = cur_free;
                prev_size = free_size;
                cur_prev = prev_occ;
                cur_next = cur_occ;
            }
            else if(fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit){
                if(free_size < prev_size || prev_size == 0){
                    cur = cur_free;
                    prev_size = free_size;
                    cur_prev = prev_occ;
                    cur_next = cur_occ;
                }
            }
            else if(fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit){
                if(free_size > prev_size){
                    cur = cur_free;
                    prev_size = free_size;
                    cur_prev = prev_occ;
                    cur_next = cur_occ;
                }
            }
        }
    }
    
    if(!cur){
        error_with_guard("Allocator_boundary_tags: failed to allocate.\n");
        throw std::bad_alloc();
    }

    if(!cur_occ && !prev_occ){
        prev_size = *reinterpret_cast<size_t*>(_trusted_memory);
    }

    size_t diff = prev_size - (size + meta_block);
    if(diff > 0 && diff < meta_block){
        size += diff;
        warning_with_guard("Allocator_boundary_tags: requested size was changed.\n");
    }

    merge(cur_prev, cur);
    merge(cur, cur_next);

    *reinterpret_cast<size_t*>(cur) = size;
    *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char*>(cur) + sizeof(size_t)) = this;

    void *result = reinterpret_cast<unsigned char*>(cur) + meta_block;

    std::vector<allocator_test_utils::block_info> b_info = get_blocks_info();
    print_info(b_info);

    information_with_guard("Allocator_boundary_tags: available memory = " + std::to_string(get_free_size()));

    debug_with_guard("Allocator_boundary_tags: end allocate.\n");
    return result;
}

void *allocator_boundary_tags::get_first_occ() const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) 
    + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex));
}

void *allocator_boundary_tags::get_first_block() const noexcept
{
    return reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*);
}

void *allocator_boundary_tags::get_next(void *block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(block) + sizeof(allocator*) + sizeof(size_t) + sizeof(void*));
}

void *allocator_boundary_tags::get_end() const noexcept
{
    return reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*) + *reinterpret_cast<size_t*>(_trusted_memory);
}

void allocator_boundary_tags::merge(void *prev, void *next) noexcept
{
    if(prev){
        void **new_next = reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(prev) + sizeof(size_t) + sizeof(allocator*) + sizeof(void*));
        *new_next = next;
    }
    else{
        set_occ_block(next);
    }

    if(next){
        void **new_prev = reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(next) + sizeof(size_t) + sizeof(allocator*));
        *new_prev = prev;
    }
}

void allocator_boundary_tags::set_occ_block(void *block) const noexcept
{
    *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) 
    + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex)) = block;
}

size_t allocator_boundary_tags::get_free_size() const noexcept
{
    void *cur = get_first_occ();
    if(!cur){
        return *reinterpret_cast<size_t*>(_trusted_memory);
    }

    void *next = nullptr;

    size_t occ_size = 0;
    auto meta = sizeof(size_t) + sizeof(allocator*) + 2*sizeof(void*);

    while(cur){
        occ_size += *reinterpret_cast<size_t*>(cur) + meta;
        cur = get_next(cur);
    }
    return *reinterpret_cast<size_t*>(_trusted_memory) - occ_size;
}

void allocator_boundary_tags::deallocate(void *at)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard("Allocator_boundary_tags: start deallocate.\n");

    auto meta = 2*sizeof(void*) + sizeof(size_t) + sizeof(allocator*);
    unsigned char *block = reinterpret_cast<unsigned char*>(at) - meta;

    if(get_block_allocator(block) != this){
        error_with_guard("Allocator_boundary_tags: wrong allocator.\n");
        throw std::logic_error("Allocator_boundary_tags: wrong allocator.\n");
    }

    void *prev = get_prev(block);
    void *next = get_next(block);

    merge(prev, next);
    clear(block);

    information_with_guard("Allocator_boundary_tags: free memory - " + std::to_string(get_free_size()) + ".\n");
    std::vector<allocator_test_utils::block_info> blocks_info = get_blocks_info();
    blocks_info = get_blocks_info();
    print_info(blocks_info);
    debug_with_guard("Allocator_boundary_tags: end deallocate.\n");
}

void allocator_boundary_tags::clear(void *block) const noexcept
{
    unsigned char *ptr = reinterpret_cast<unsigned char*>(block);
    size_t *size = reinterpret_cast<size_t*>(block);
    size = nullptr;

    ptr += sizeof(size_t);
    allocator **all = reinterpret_cast<allocator**>(ptr);
    all = nullptr;

    ptr += sizeof(allocator*);
    void **prev = reinterpret_cast<void**>(ptr);
    prev = nullptr;

    ptr += sizeof(void*);
    void **next = reinterpret_cast<void**>(ptr);
    next = nullptr;
}

void *allocator_boundary_tags::get_prev(void *block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(block) + sizeof(allocator*) + sizeof(size_t));
}

allocator *allocator_boundary_tags::get_block_allocator(void *block) const noexcept
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char*>(block) + sizeof(size_t));
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*)) = mode;
}

inline allocator_with_fit_mode::fit_mode allocator_boundary_tags::get_fit_mode()
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t));
}

inline allocator *allocator_boundary_tags::get_allocator() const
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t));
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> blocks_info;

    void *cur = get_first_occ();
    if(!cur){
        allocator_test_utils::block_info block;
        block.block_size = *reinterpret_cast<size_t*>(_trusted_memory);
        block.is_block_occupied = false;
        blocks_info.push_back(block);
        return blocks_info;
    }

    void *prev = nullptr;
    auto meta = 2*sizeof(void*) + sizeof(size_t) + sizeof(allocator*);

    while(cur){
        if((!prev && cur != get_first_block()) || (prev && (reinterpret_cast<unsigned char*>(prev) + meta + *reinterpret_cast<size_t*>(prev) != cur))){
            size_t size;
            if(!prev){
                size = reinterpret_cast<unsigned char *>(cur) - (reinterpret_cast<unsigned char*>(get_first_block()));
            }
            else{
                size = reinterpret_cast<unsigned char *>(cur) - (reinterpret_cast<unsigned char*>(prev) + meta + *reinterpret_cast<size_t*>(prev));
            }
            allocator_test_utils::block_info block;
            block.block_size = size;
            block.is_block_occupied = false;
            blocks_info.push_back(block);
        }
        allocator_test_utils::block_info block;
        block.block_size = *reinterpret_cast<size_t*>(cur);
        block.is_block_occupied = true;
        blocks_info.push_back(block);
        prev = cur;
        cur = get_next(cur);
    }

    if(prev != get_end() && prev){
        size_t size = reinterpret_cast<unsigned char*>(get_end()) - (reinterpret_cast<unsigned char*>(prev) + meta + *reinterpret_cast<size_t*>(prev));
        allocator_test_utils::block_info block;
        block.block_size = size;
        block.is_block_occupied = false;
        if(size){
            blocks_info.push_back(block);
        }
    }
    return blocks_info;
}

void allocator_boundary_tags::print_info(std::vector<allocator_test_utils::block_info> blocks_info) const noexcept
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

inline logger *allocator_boundary_tags::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*));
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "Allocator_boundary_tags";
}

std::mutex &allocator_boundary_tags::get_mutex() const noexcept
{
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode));
}