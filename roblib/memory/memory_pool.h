// memory_pool.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/03 15:26:49 PST

#pragma once

#include <stddef.h>
#include <stdbool.h>

// --- Structures ---

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

// --- API Functions ---

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
