// memory_pool.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/03 15:26:49 PST

#pragma once

#include <stddef.h>
#include <stdint.h>
//Memory Policy.... what are options?
// stdlib vs allocator?
// maybe always use an allocator, and the default allocator just makes malloc calls?
// To start, HashMap will manage the lifecycle of its allocator (create, use, destroy).
// inital memory manager algorithm will be wasteful with memory but more efficent that
// small individual mallocs.
typedef enum MemPolicyType: uint8_t {
    MEM_POLICY_NONE,
    MEM_POLICY_MALLOC_OWN,
    MEM_POLICY_MALLOC_SHARED,
    MEM_POLICY_ALLOCATOR_OWN,
    MEM_POLICY_ALLOCATOR_SHARED,
    MEM_POLICY_NULL,
} MemPolicyType;

// --- Structures ---

// 56 bytes
typedef struct MemPolicy {
    void * context;
    void * (*alloc)(   void * context, size_t num_bytes );
    void * (*calloc)(  void * context, size_t element_count, size_t element_size);
    void * (*realloc)( void * context, void * pointer, size_t old_byte_count, size_t new_byte_count );
    void   (*free)(    void * context, void * pointer );
    void   (*free_context)(void * context );
    MemPolicyType policy_type;
} MemPolicy;

// A single page of memory.
// 32 bytes
typedef struct MemoryPage {
    void *start;                 // Pointer to the start of the page's memory.
    size_t size;                 // Total size of this page.
    size_t used;                 // How much of this page is currently used.
    struct MemoryPage *next;     // Next page in the linked list.
} MemoryPage;

// A memory pool that manages multiple pages.
// 16 bytes
typedef struct MemoryPool {
    MemoryPage *pages_head;      // Head of the linked list of all allocated pages.
    size_t default_page_size;    // Default size for new pages.
} MemoryPool;

// static constexpr size_t DEFAULT_PAGE_SIZE = 4096;


static constexpr size_t DEFAULT_PAGE_SIZE = 64L * 1024 * 1024; // 64 MB

extern const MemPolicy MEM_DEFAULT_MALLOC_POLICY;
extern const MemPolicy MEM_DEFAULT_ALLOCATOR_POLICY;
extern const MemPolicy NULL_MEM_POLICY;
// on my system, MAX_MALLOC_BYTES = 1'099'511'627'776; // 16^10
// 2^34, ~17.2GB which is more than my computer ram
// let's clamp to something more reasonable like 2^33 bytes, about 8.6GB, half my RAM
[[maybe_unused]]
static constexpr size_t MAX_MALLOC_BYTES = 1L << 33;  // 2^33, ~8.6GB, half my RAM

// --- API Functions ---

// Calls the mem_policy's `alloc` function
void * mem_alloc_bytes( MemPolicy mem_policy,  size_t num_bytes);
void * mem_calloc_bytes( MemPolicy mem_policy,  size_t element_count, size_t element_size);
void * mem_realloc_bytes( MemPolicy mem_policy, void * pointer,  size_t old_byte_count, size_t new_byte_count);
char * mem_strdup(MemPolicy mem_policy, char const * string) ;
// Calls the mem_policy's `free` function
void  mem_free_bytes( MemPolicy mem_policy, void * bytes);

MemPolicy mem_create_default_allocator( size_t default_page_size);

bool mem_equals_MemPolicy( MemPolicy o1,  MemPolicy o2);
/**
 * @brief Creates a new memory pool.
 *
 * @param default_page_size The default size for new pages allocated by the pool.
 *                          If 0, a reasonable default (e.g., 4KB) is used.
 * @return A pointer to the newly created MemoryPool, or NULL on failure.
 */
MemoryPool *pool_create(size_t default_page_size);

/**
 * @brief Destroys a memory pool and frees all associated memory.
 *
 * @param pool The memory pool to destroy.
 */
void pool_destroy(MemoryPool *pool);

/**
 * @brief Allocates a block of memory from the pool.
 *
 * This function will attempt to find space in existing pages. If no space is
 * available, it will allocate a new page. For allocations larger than the
 * default page size, a new page of the required size will be allocated.
 *
 * @param pool The memory pool to allocate from.
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory, or NULL on failure.
 */
void *pool_alloc(MemoryPool *pool, size_t size);

/**
 * @brief Resets the memory pool, effectively "freeing" all memory.
 *
 * This function does not actually free the allocated pages but marks them as
 * available for new allocations. This is a very fast way to "clear" the pool.
 *
 * @param pool The memory pool to reset.
 */
void pool_reset(MemoryPool *pool);

/**
 * @brief Frees a specific allocation.
 *
 * Note: In this simple pool allocator, individual "free" operations are complex
 * and can lead to fragmentation. The primary "free" mechanism is `pool_reset`
 * or `pool_destroy`. This function is a placeholder for a more advanced
 * implementation and currently does nothing.
 *
 * @param pool The memory pool.
 * @param ptr Pointer to the memory to free.
 */
void pool_free(MemoryPool *pool, void *ptr);
