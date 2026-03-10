// memory_pool.c
//
// Copyright (c) Rob Ross 2026.
//
//

#include "memory_pool.h"
#include <stdlib.h>
#include <string.h>



// --- Helper Functions ---

static MemoryPage *create_page(size_t size) {
    MemoryPage *page = (MemoryPage *)calloc(1, sizeof(MemoryPage));
    if (!page) return nullptr;

    page->start = calloc(1, size);
    if (!page->start) {
        free(page);
        return nullptr;
    }

    page->size = size;
    page->used = 0;
    page->next = nullptr;
    return page;
}

static void destroy_page(MemoryPage *page) {
    if (page) {
        free(page->start);
        free(page);
    }
}

// --- API Implementation ---

//// ------------------------------------------------------------
////
////    Default memory policy functions
////
//// ------------------------------------------------------------
void * mem_alloc_bytes(const MemPolicy mem_policy, const size_t num_bytes) {
    return mem_policy.alloc(mem_policy.context, num_bytes);
}

void  mem_free_bytes(const MemPolicy mem_policy, void * bytes) {
    mem_policy.free(mem_policy.context, bytes);
}

void * mem_mempolicy_default_malloc( void* context, size_t num_bytes ) {
    // todo - do we need a mempolicy calloc method separate from malloc for performance?
    return calloc(1, num_bytes);
}

void mem_mempolicy_default_free( void* context, void * bytes ) {
    free( bytes) ;
}

void * mem_mempolicy_default_allocator_alloc( void* context, size_t num_bytes ) {
    MemoryPool *pool = context;
    return pool_alloc(pool, num_bytes);
}

void  mem_mempolicy_default_allocator_free( void * context, void * bytes ) {
    MemoryPool *pool = context;
    pool_free(pool, bytes);
}

const MemPolicy MEM_DEFAULT_MALLOC_POLICY = (MemPolicy){
    .context = nullptr,
    .policy_type = MEM_POLICY_MALLOC_OWN,
    .alloc = mem_mempolicy_default_malloc,
    .free = mem_mempolicy_default_free,
};

const MemPolicy MEM_DEFAULT_ALLOCATOR_POLICY = (MemPolicy){
    .context = nullptr,  // context gets filled in by actual memory_pool
    .policy_type = MEM_POLICY_ALLOCATOR_OWN,
    .alloc = mem_mempolicy_default_allocator_alloc,
    .free = mem_mempolicy_default_allocator_free
};




MemoryPool *pool_create(size_t default_page_size) {
    default_page_size = default_page_size > DEFAULT_PAGE_SIZE ? default_page_size : DEFAULT_PAGE_SIZE;
    MemoryPool *pool = (MemoryPool *)calloc(1, sizeof(MemoryPool));
    if (!pool) return nullptr;

    pool->default_page_size = default_page_size;
    pool->pages_head = nullptr;
    return pool;
}

void pool_destroy(MemoryPool *pool) {
    if (!pool) return;

    MemoryPage *current = pool->pages_head;
    while (current) {
        MemoryPage *next = current->next;
        destroy_page(current);
        current = next;
    }
    free(pool);
}

void *pool_alloc(MemoryPool *pool, size_t size) {
    if (!pool || size == 0) return nullptr;

    // Try to allocate from existing pages
    MemoryPage *current = pool->pages_head;
    while (current) {
        if (current->size - current->used >= size) {
            void *ptr = (char *)current->start + current->used;
            current->used += size;
            return ptr;
        }
        current = current->next;
    }

    // No suitable page found, allocate a new one
    size_t new_page_size = (size > pool->default_page_size) ? size : pool->default_page_size;
    MemoryPage *new_page = create_page(new_page_size);
    if (!new_page) return nullptr;

    // Add new page to the head of the list
    new_page->next = pool->pages_head;
    pool->pages_head = new_page;

    // Allocate from the new page
    void *ptr = new_page->start;
    new_page->used = size;
    return ptr;
}

void pool_reset(MemoryPool *pool) {
    if (!pool) return;

    MemoryPage *current = pool->pages_head;
    while (current) {
        current->used = 0;
        current = current->next;
    }
}

void pool_free(MemoryPool *pool, void *ptr) {
    // In this simple pool allocator, individual frees are not supported.
    // The memory is freed when the pool is destroyed or reset.
    (void)pool;
    (void)ptr;
}
