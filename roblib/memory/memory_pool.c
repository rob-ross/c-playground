// memory_pool.c
//
// Copyright (c) Rob Ross 2026.
//
//

#include "memory_pool.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PAGE_SIZE 4096

// --- Helper Functions ---

static MemoryPage *create_page(size_t size) {
    MemoryPage *page = (MemoryPage *)malloc(sizeof(MemoryPage));
    if (!page) return NULL;

    page->start = malloc(size);
    if (!page->start) {
        free(page);
        return NULL;
    }

    page->size = size;
    page->used = 0;
    page->next = NULL;
    return page;
}

static void destroy_page(MemoryPage *page) {
    if (page) {
        free(page->start);
        free(page);
    }
}

// --- API Implementation ---

MemoryPool *pool_create(size_t default_page_size) {
    MemoryPool *pool = (MemoryPool *)malloc(sizeof(MemoryPool));
    if (!pool) return NULL;

    pool->default_page_size = (default_page_size > 0) ? default_page_size : DEFAULT_PAGE_SIZE;
    pool->pages_head = NULL;
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
    if (!pool || size == 0) return NULL;

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
    if (!new_page) return NULL;

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
