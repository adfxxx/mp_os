#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <cstring>
#include <mutex>

class allocator_boundary_tags final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    
    void *_trusted_memory;

public:
    
    ~allocator_boundary_tags() override;
    
    allocator_boundary_tags(allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags &operator=(allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags(allocator_boundary_tags &&other) noexcept;
    
    allocator_boundary_tags &operator=(allocator_boundary_tags &&other) noexcept;

public:
    
    explicit allocator_boundary_tags(
        size_t space_size,
        allocator *parent_allocator = nullptr,
        logger *logger = nullptr,
        allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *allocate(size_t value_size,size_t values_count) override;
    
    void deallocate(void *at) override;

public:
    
    inline void set_fit_mode(allocator_with_fit_mode::fit_mode mode) override;
    inline allocator_with_fit_mode::fit_mode get_fit_mode();

private:
    
    inline allocator *get_allocator() const override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

private:
    
    inline logger *get_logger() const override;

private:
    
    inline std::string get_typename() const noexcept override;

private:

    std::mutex &get_mutex() const noexcept;
    void *get_first_occ() const noexcept;
    void *get_first_block() const noexcept;
    void *get_next(void *block) const noexcept;
    void *get_prev(void *block) const noexcept;
    void *get_end() const noexcept;
    void merge(void *prev, void *next) noexcept;
    void set_occ_block(void *block) const noexcept;
    allocator *get_block_allocator(void *block) const noexcept;
    size_t get_free_size() const noexcept;
    void clear(void *block) const noexcept;
    void print_info(std::vector<allocator_test_utils::block_info> blocks_info) const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H