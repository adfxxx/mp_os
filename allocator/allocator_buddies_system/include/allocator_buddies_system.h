#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <mutex>
#include <cstring>

class allocator_buddies_system final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    
    void *_trusted_memory;

public:
    
    ~allocator_buddies_system() override;
    
    allocator_buddies_system(allocator_buddies_system const &other) = delete;
    
    allocator_buddies_system &operator=(allocator_buddies_system const &other) = delete;
    
    allocator_buddies_system(allocator_buddies_system &&other) noexcept;
    
    allocator_buddies_system &operator=(allocator_buddies_system &&other) noexcept;

public:
    
    explicit allocator_buddies_system(
        size_t space_size_power_of_two,
        allocator *parent_allocator = nullptr,
        logger *logger = nullptr,
        allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *allocate(size_t value_size,size_t values_count) override;
    
    void deallocate(void *at) override;

public:
    
    inline void set_fit_mode(allocator_with_fit_mode::fit_mode mode) override;

private:
    
    inline allocator *get_allocator() const override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

private:
    
    inline logger *get_logger() const override;

private:
    
    inline std::string get_typename() const noexcept override;
    
private:

    void make_meta(unsigned char *block, signed char degree, void *prev, void *next) const noexcept;
    size_t get_meta_size() const noexcept;
    unsigned char get_degree(size_t num) const noexcept;
    size_t get_free_memory() const noexcept;
    void *get_first_free() const noexcept;
    signed char block_degree(void *block) const noexcept;
    void *get_next(void *block) const noexcept;
    inline allocator_with_fit_mode::fit_mode get_fit_mode() const noexcept;
    void *split(void *block, unsigned char &degree) const noexcept;
    void *get_buddy(void *block, size_t size) const noexcept;
    void create_free_block(void *block, signed char degree, void *prev) const noexcept;
    void connect(void *prev, void *next) const noexcept;
    void *get_prev(void *block) const noexcept;
    void make_occ_meta(unsigned char *block, signed char degree) const noexcept;
    void set_first_block(void *block) const noexcept;
    void cut_free_space(size_t &size) const noexcept;
    void print_info(std::vector<allocator_test_utils::block_info> blocks_info) const noexcept;
    unsigned char *get_occ_block_start(void *block) const noexcept;
    void merge(void *prev, void *next) const noexcept;
    void *find_block(void *block, void *end) const noexcept;
    void add_free_space(size_t space) const noexcept;
    std::mutex &get_mutex() const noexcept;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
