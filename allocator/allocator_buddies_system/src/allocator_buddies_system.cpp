#include <not_implemented.h>

#include "../include/allocator_buddies_system.h"

allocator_buddies_system::~allocator_buddies_system()
{
    debug_with_guard("Allocator_buddies_system: start destructor.\n");
    logger *_logger = get_logger();
    allocator::destruct(&get_mutex());
    deallocate_with_guard(_trusted_memory);
    if(_logger){
        _logger->debug("Allocator_buddies_system: end destructor.\n");
    }
}

allocator_buddies_system::allocator_buddies_system(allocator_buddies_system &&other) noexcept
{
    logger *_logger = get_logger();
    if(_logger){
        _logger->debug("Allocator_buddies_system: start move constructor.\n");
    }

    if(_trusted_memory){
        deallocate_with_guard(_trusted_memory);
    }
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    if(_logger){
        _logger->debug("Allocator_buddies_system: end move constructor.\n");
    }
}

allocator_buddies_system &allocator_buddies_system::operator=(allocator_buddies_system &&other) noexcept
{
    logger *_logger = get_logger();
    if(_logger){
        _logger->debug("Allocator_buddies_system: start move operator.\n");
    }

    if(this != &other){
        if(_trusted_memory){
            deallocate_with_guard(_trusted_memory);
        }
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }

    if(_logger){
        _logger->debug("Allocator_buddies_system: end move operator.\n");
    }
    return *this;
}

allocator_buddies_system::allocator_buddies_system(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger_,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if(logger_){
        logger_->debug("Allocator_buddies_system: start constructor.\n");
    }

    auto meta_block = sizeof(signed char) + 2*sizeof(void*);
    if(space_size < get_degree(meta_block)){
        if(logger_){
            logger_->error("Allocator_buddies_system: space size is wrong.\n");
        }
        throw std::logic_error("Allocator_buddies_system: space size is wrong.\n");
    }

    auto meta = get_meta_size();
    auto memory = 1 << space_size;

    try{
        if(parent_allocator){
            _trusted_memory = parent_allocator->allocate(memory + meta,1);
        }
        else{
            _trusted_memory = ::operator new(memory + meta);
        }
    }
    catch(const std::bad_alloc ex){
        if(logger_){
            logger_->error("Allocator_buddies_system: failed to allocate.\n");
        }
        throw ex;
    }

    unsigned char *result = reinterpret_cast<unsigned char*>(_trusted_memory);
    result++;

    *result = static_cast<signed char>(space_size);
    result++;

    *reinterpret_cast<size_t*>(result) = memory;
    result += sizeof(size_t);

    *reinterpret_cast<allocator**>(result) = parent_allocator;
    result += sizeof(allocator*);

    *reinterpret_cast<logger**>(result) = logger_;
    result += sizeof(logger*);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(result) = allocate_fit_mode;
    result += sizeof(allocator_with_fit_mode::fit_mode);

    allocator::construct(reinterpret_cast<std::mutex*>(result));
    result += sizeof(std::mutex);

    *reinterpret_cast<void**>(result) = result + sizeof(void*);
    result += sizeof(void*);

    make_meta(result, static_cast<signed char>(space_size), nullptr, nullptr);

    if(logger_){
        logger_->debug("Allocator_buddies_system: end constructor.\n");
    }
}

void allocator_buddies_system::make_meta(unsigned char *block, signed char degree, void *prev, void *next) const noexcept
{
    *reinterpret_cast<signed char*>(block) = -abs(degree);
    *reinterpret_cast<void**>(block + sizeof(signed char)) = prev;
    *reinterpret_cast<void**>(block + sizeof(signed char) + sizeof(void*)) = next;
}

size_t allocator_buddies_system::get_meta_size() const noexcept
{
    return sizeof(unsigned char) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*);
}

unsigned char allocator_buddies_system::get_degree(size_t num) const noexcept
{
    unsigned char dgr = 0;
    while (num >>= 1){
        ++dgr;
    }
    return dgr;
}

[[nodiscard]] void *allocator_buddies_system::allocate(size_t value_size, size_t values_count)
{
    std::lock_guard<std::mutex>lock(get_mutex());
    debug_with_guard("Allocator_buddies_system: start allocate.\n");

    auto size = value_size*values_count + sizeof(signed char) + sizeof(unsigned char*); // + occ meta
    unsigned char degree = get_degree(size);
    unsigned char free_meta_dgr = get_degree(sizeof(signed char) + 2*sizeof(void*));
    unsigned char occ_meta_dgr = get_degree(sizeof(signed char) + sizeof(unsigned char*));

    if(degree < (free_meta_dgr > occ_meta_dgr ? free_meta_dgr : occ_meta_dgr)){
        degree = (free_meta_dgr > occ_meta_dgr ? free_meta_dgr : occ_meta_dgr);
        warning_with_guard("Allocator_buddies_system: size was changed.\n");
    }

    if(size > get_free_memory()){
        error_with_guard("Allocator_buddies_system: there is not enough space to allocate.\n");
        throw std::bad_alloc();
    }

    allocator_with_fit_mode::fit_mode mode = get_fit_mode();

    void *block = nullptr;
    void *cur = get_first_free();
    void *prev = nullptr;

    if(mode == allocator_with_fit_mode::fit_mode::first_fit){
        while(cur){
            if(abs(block_degree(cur)) >= degree){
                block = cur;
                break;
            }
            cur = get_next(cur);
        }
    }
    else if(mode == allocator_with_fit_mode::fit_mode::the_best_fit || mode == allocator_with_fit_mode::fit_mode::the_worst_fit){
        unsigned char best_dgr;
        if(mode == allocator_with_fit_mode::fit_mode::the_best_fit){
            best_dgr = abs(get_degree(get_free_memory()));
        }
        else{
            best_dgr = 0;
        }

        while(cur){
            bool flag;
            if(mode == allocator_with_fit_mode::fit_mode::the_best_fit){
                flag = abs(block_degree(cur)) <= best_dgr;
            }
            else{
                flag = abs(block_degree(cur)) >= best_dgr;
            }
            
            if(best_dgr >= degree && flag){
                block = cur;
            }
            cur = get_next(cur);
        }
    }

    if(!block){
        error_with_guard("Allocator_buddies_system: failed to allocate.\n");
        throw std::bad_alloc();
    }

    block = split(block, degree);

    cut_free_space(size);

    information_with_guard("Allocator_buddies_system: free space size - " + std::to_string(get_free_memory()));
    std::vector<allocator_test_utils::block_info> b_info = get_blocks_info();
    print_info(b_info);
    debug_with_guard("Allocator_buddies_system: end allocate.\n");

    return reinterpret_cast<unsigned char*>(block) + sizeof(signed char) + sizeof(unsigned char*);
}

size_t allocator_buddies_system::get_free_memory() const noexcept
{
    return *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(_trusted_memory) + 1);
}

void *allocator_buddies_system::get_first_free() const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode)+
    sizeof(std::mutex) + 1);
}

signed char allocator_buddies_system::block_degree(void *block) const noexcept
{
    return static_cast<signed char>(*reinterpret_cast<unsigned char*>(block));
}

void *allocator_buddies_system::get_next(void *block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(block) + sizeof(signed char) + sizeof(void*));
}

void *allocator_buddies_system::split(void *block, unsigned char &degree) const noexcept
{
    void *cur = block;
    void *buddy = nullptr;

    while(abs(block_degree(cur)) - 1 > degree){
        buddy = get_buddy(cur, 1 << (abs(block_degree(cur)) - 1));
        create_free_block(buddy, abs(block_degree(cur)) - 1, cur);
    }
    connect(get_prev(cur), get_next(cur));
    make_occ_meta(reinterpret_cast<unsigned char*>(cur), abs(block_degree(cur)));

    return block;
}

void *allocator_buddies_system::get_buddy(void *block, size_t size) const noexcept
{
    unsigned char *start = reinterpret_cast<unsigned char*>(reinterpret_cast<unsigned char*>(_trusted_memory) + get_meta_size());
    return ((reinterpret_cast<unsigned char*>(block) - start) ^ (size)) + start;
}

void allocator_buddies_system::create_free_block(void *block, signed char degree, void *prev) const noexcept
{
    void *next = get_next(prev);
    
    if(next){
        make_meta(reinterpret_cast<unsigned char*>(next), block_degree(next), block, get_next(next));
    }
    make_meta(reinterpret_cast<unsigned char*>(block), degree, prev, next);
    make_meta(reinterpret_cast<unsigned char*>(prev), degree, get_prev(prev), block);
}

void allocator_buddies_system::connect(void *prev, void *next) const noexcept
{
    if(prev){
        make_meta(reinterpret_cast<unsigned char*>(prev), block_degree(prev), get_prev(prev), next);
    }
    else{
        set_first_block(next);
    }

    if(next){
        make_meta(reinterpret_cast<unsigned char*>(next), block_degree(next), prev, get_next(next));
    }
}

void *allocator_buddies_system::get_prev(void *block) const noexcept
{
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(block) + sizeof(signed char));
}

void allocator_buddies_system::make_occ_meta(unsigned char *block, signed char degree) const noexcept
{
    *reinterpret_cast<signed char*>(block) = abs(degree);
    *reinterpret_cast<unsigned char**>(block + sizeof(signed char)) = reinterpret_cast<unsigned char*>(_trusted_memory); //memory start
}

void allocator_buddies_system::set_first_block(void *block) const noexcept
{
    *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(_trusted_memory) + get_meta_size() - sizeof(void)) = block;
}

void allocator_buddies_system::cut_free_space(size_t &size) const noexcept
{
    *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(_trusted_memory) + 1) = get_free_memory() - size;
}

void allocator_buddies_system::deallocate(void *at)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    debug_with_guard("Allocator_buddies_system: start deallocate.\n");

    void *block = reinterpret_cast<unsigned char*>(at) - sizeof(signed char) - sizeof(unsigned char*);

    if(get_occ_block_start(block) != reinterpret_cast<unsigned char*>(_trusted_memory)){//memory start
        error_with_guard("Allocator_buddies_system: wrong allocator.\n");
        throw std::logic_error("Allocator_buddies_system: wrong allocator.\n");
    }

    merge(block, reinterpret_cast<unsigned char*>(_trusted_memory) + get_meta_size() + (1 << get_degree(get_free_memory())));

    add_free_space(1 << abs(block_degree(block)));

    information_with_guard("Allocator_buddies_system: free memory size = " + std::to_string((get_free_memory())));
    std::vector<allocator_test_utils::block_info> b_info = get_blocks_info();
    print_info(b_info);
    debug_with_guard("Allocator_buddies_system: end deallocate.\n");
}

void allocator_buddies_system::add_free_space(size_t space) const noexcept
{
    *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(_trusted_memory) + 1) = get_free_memory() + space;
}

void allocator_buddies_system::merge(void *prev, void *next) const noexcept
{
    unsigned char degree = abs(block_degree(prev));
    unsigned char *buddy = reinterpret_cast<unsigned char*>(get_buddy(prev, 1 << degree));

    if(buddy == reinterpret_cast<unsigned char*>(next)){
        return;
    }
    signed char buddy_dgr = block_degree(buddy);

    if(buddy_dgr > 0){
        unsigned char *prev_free = reinterpret_cast<unsigned char*>(find_block(prev, next));
        void *next_2 = nullptr;
        if(!prev_free){
            next_2 = get_first_free();
            set_first_block(prev);
        }
        else{
            next_2 = get_next(prev_free);
            make_meta(reinterpret_cast<unsigned char*>(prev_free), block_degree(prev_free), get_prev(prev_free), prev);
        }
        make_meta(reinterpret_cast<unsigned char*>(next_2), block_degree(next_2), prev, get_next(next_2));
        make_meta(reinterpret_cast<unsigned char*>(prev), block_degree(prev), prev_free, next_2);
        return;
    }

    if(buddy < reinterpret_cast<unsigned char*>(prev)){
        make_meta(buddy, abs(block_degree(buddy)) + 1, get_prev(buddy), get_next(buddy));
        merge(buddy, next);
    }
    else{
        make_meta(reinterpret_cast<unsigned char*>(prev), block_degree(prev) + 1, get_prev(buddy), buddy);
        merge(prev, next);
    }
}

void *allocator_buddies_system::find_block(void *block, void *end) const noexcept
{
    unsigned char *end_2 = reinterpret_cast<unsigned char*>(end);
    unsigned char *cur = reinterpret_cast<unsigned char*>(_trusted_memory) + get_meta_size();
    unsigned char *prev = nullptr;

    while (cur != end){
        if(cur == block){
            return prev;
        }
        if(block_degree(cur) > 0){
            prev = cur;
        }
        cur = cur + (1 << abs(block_degree(cur)));
    }
    return nullptr;
}

unsigned char *allocator_buddies_system::get_occ_block_start(void *block) const noexcept
{
    return *reinterpret_cast<unsigned char**>(reinterpret_cast<unsigned char*>(block) + sizeof(signed char));
}

inline void allocator_buddies_system::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) + 1) = mode;
}

inline allocator_with_fit_mode::fit_mode allocator_buddies_system::get_fit_mode() const noexcept
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + 1);
}

inline allocator *allocator_buddies_system::get_allocator() const
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t) + 1);
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> blocks_info;

    unsigned char *cur = reinterpret_cast<unsigned char*>(_trusted_memory) + get_meta_size();
    unsigned char *memory_end = cur + (1 << block_degree(_trusted_memory));

    block_info info;
    while(cur != memory_end){
        info.is_block_occupied = block_degree(cur) > 0 ? true : false;
        info.block_size = 1 << abs(block_degree(cur));
        blocks_info.push_back(info);

        cur += (1 << abs(block_degree(cur)));
    }
    return blocks_info;
}

void allocator_buddies_system::print_info(std::vector<allocator_test_utils::block_info> blocks_info) const noexcept
{
    std::string info = "blocks info: ";
    for(auto block : blocks_info){
        if(block.is_block_occupied){
            info += "occupied, ";
        }
        else{
            info += "available, ";
        }
        info += "size = " + std::to_string(block.block_size) += ";";
    }
    debug_with_guard(info);
}

inline logger *allocator_buddies_system::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + 1);
}

inline std::string allocator_buddies_system::get_typename() const noexcept
{
    return "Allocator_buddies_system";
}

std::mutex &allocator_buddies_system::get_mutex() const noexcept
{
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + 1);
}