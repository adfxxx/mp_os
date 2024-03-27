#include <not_implemented.h>
#include <utility>

#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap(logger *logger)
{
    trace_with_guard("Start constructor.\n");
    _logger = logger;
    trace_with_guard("End constructor.\n");
}

allocator_global_heap::~allocator_global_heap()
{
    trace_with_guard("Start destructor.\n");
    trace_with_guard("End destructor.\n");
}

allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept
{
    trace_with_guard("Start move constructor.\n");
    _logger = std::exchange(other._logger, nullptr);
    trace_with_guard("End move constructor.\n");
}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept
{
    trace_with_guard("Start move operator\n");
    if(this == &other){
        return *this;
    }
    std::swap(_logger, other._logger);
    trace_with_guard("End move operator\n");
    return *this;
}

[[nodiscard]] void *allocator_global_heap::allocate(size_t value_size, size_t values_count)
{
    debug_with_guard("Start allocate\n");

    block_size_t need_size = value_size*values_count;
    block_size_t meta_size = sizeof(allocator*) + sizeof(size_t);

    block_size_t total_size = need_size + meta_size;

    block_pointer_t new_block = ::operator new(total_size);
    if(new_block == nullptr){
        error_with_guard("Error in allocation: didn't allocate.\n");
        throw std::bad_alloc();
    }

    block_pointer_t ptr = new_block;

    *(reinterpret_cast<allocator**>(ptr)) = this;
    ptr += sizeof(allocator*);
    *(reinterpret_cast<size_t*>(ptr)) = need_size;

    block_pointer_t result = reinterpret_cast<uint8_t*>(new_block) + meta_size;

    debug_with_guard("End allocate.\n");
    return result;
}

void allocator_global_heap::deallocate(void *at)
{
    debug_with_guard("Start deallocate.\n");

    if(at == nullptr){
        return;
    }

    block_size_t meta_size = sizeof(allocator*) + sizeof(size_t);
    block_pointer_t block = reinterpret_cast<uint8_t*>(at) - meta_size;

    if((*(reinterpret_cast<allocator**>(block))) != this){
        if(get_logger()){
            _logger->error("Error in deallocation: wrong allocator.\n");
        }
        throw std::logic_error("wrong allocator\n");
    }

    block_size_t size = *reinterpret_cast<size_t*>(reinterpret_cast<uint8_t*>(block) + sizeof(size_t));

    unsigned char *bytes = reinterpret_cast<unsigned char*>(at);
    std::string arr_bytes;
    for(block_size_t i = 0; i < size; i++){
        arr_bytes += std::to_string(*bytes);
        bytes += sizeof(unsigned char);
        arr_bytes += ' ';
    }

    if(get_logger()){
        if(size != 0){
            _logger->debug("Block: " + arr_bytes + "\n");
        }
        else{
            _logger->debug("Empty block.\n");
        }
    }

    ::operator delete(block);
    debug_with_guard("End deallocate.\n");
}

inline logger *allocator_global_heap::get_logger() const
{
    return _logger;
}

inline std::string allocator_global_heap::get_typename() const noexcept
{
    trace_with_guard("Start get_typename.\n");
    trace_with_guard("End get_typename.\n");
    return "allocator_global_heap";
}