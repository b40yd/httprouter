#include "hr_palloc.h"


static inline void *hr_palloc_small(hr_pool_t *pool, size_t size,
    hr_uint_t align);
static void *hr_palloc_block(hr_pool_t *pool, size_t size);
static void *hr_palloc_large(hr_pool_t *pool, size_t size);


void *
hr_alloc(size_t size)
{
    return malloc(size);
}


void *
hr_calloc(size_t size)
{
    void  *p;

    p = hr_alloc(size);

    if (p) {
        hr_memzero(p, size);
    }

    return p;
}

void *
hr_memalign(size_t alignment, size_t size)
{
    void  *p;
    int    err;

    err = posix_memalign(&p, alignment, size);

    if (err) {
        p = NULL;
    }

    return p;
}

hr_pool_t *
hr_create_pool(size_t size)
{
    hr_pool_t  *p;

    p = hr_memalign(HR_POOL_ALIGNMENT, size);
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (hr_u_char *) p + sizeof(hr_pool_t);
    p->d.end = (hr_u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(hr_pool_t);
    p->max = (size < HR_MAX_ALLOC_FROM_POOL) ? size : HR_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->large = NULL;
    p->cleanup = NULL;

    return p;
}


void
hr_destroy_pool(hr_pool_t *pool)
{
    hr_pool_t          *p, *n;
    hr_pool_large_t    *l;
    hr_pool_cleanup_t  *c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }


    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            hr_free(l->alloc);
        }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        hr_free(p);

        if (n == NULL) {
            break;
        }
    }
}


void
hr_reset_pool(hr_pool_t *pool)
{
    hr_pool_t        *p;
    hr_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            hr_free(l->alloc);
        }
    }

    for (p = pool; p; p = p->d.next) {
        p->d.last = (hr_u_char *) p + sizeof(hr_pool_t);
        p->d.failed = 0;
    }

    pool->current = pool;
    pool->large = NULL;
}


void *
hr_palloc(hr_pool_t *pool, size_t size)
{
#if !(HR_DEBUG_PALLOC)
    if (size <= pool->max) {
        return hr_palloc_small(pool, size, 1);
    }
#endif

    return hr_palloc_large(pool, size);
}


void *
hr_pnalloc(hr_pool_t *pool, size_t size)
{
#if !(HR_DEBUG_PALLOC)
    if (size <= pool->max) {
        return hr_palloc_small(pool, size, 0);
    }
#endif

    return hr_palloc_large(pool, size);
}


static inline void *
hr_palloc_small(hr_pool_t *pool, size_t size, hr_uint_t align)
{
    hr_u_char      *m;
    hr_pool_t  *p;

    p = pool->current;

    do {
        m = p->d.last;

        if (align) {
            m = hr_align_ptr(m, HR_ALIGNMENT);
        }

        if ((size_t) (p->d.end - m) >= size) {
            p->d.last = m + size;

            return m;
        }

        p = p->d.next;

    } while (p);

    return hr_palloc_block(pool, size);
}


static void *
hr_palloc_block(hr_pool_t *pool, size_t size)
{
    hr_u_char      *m;
    size_t       psize;
    hr_pool_t  *p, *new;

    psize = (size_t) (pool->d.end - (hr_u_char *) pool);

    m = hr_memalign(HR_POOL_ALIGNMENT, psize);
    if (m == NULL) {
        return NULL;
    }

    new = (hr_pool_t *) m;

    new->d.end = m + psize;
    new->d.next = NULL;
    new->d.failed = 0;

    m += sizeof(hr_pool_data_t);
    m = hr_align_ptr(m, HR_ALIGNMENT);
    new->d.last = m + size;

    for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool->current = p->d.next;
        }
    }

    p->d.next = new;

    return m;
}


static void *
hr_palloc_large(hr_pool_t *pool, size_t size)
{
    void              *p;
    hr_uint_t         n;
    hr_pool_large_t  *large;

    p = hr_alloc(size);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = hr_palloc_small(pool, sizeof(hr_pool_large_t), 1);
    if (large == NULL) {
        hr_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


void *
hr_pmemalign(hr_pool_t *pool, size_t size, size_t alignment)
{
    void              *p;
    hr_pool_large_t  *large;

    p = hr_memalign(alignment, size);
    if (p == NULL) {
        return NULL;
    }

    large = hr_palloc_small(pool, sizeof(hr_pool_large_t), 1);
    if (large == NULL) {
        hr_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


hr_int_t
hr_pfree(hr_pool_t *pool, void *p)
{
    hr_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            hr_free(l->alloc);
            l->alloc = NULL;

            return HR_OK;
        }
    }

    return HR_DECLINED;
}


void *
hr_pcalloc(hr_pool_t *pool, size_t size)
{
    void *p;

    p = hr_palloc(pool, size);
    if (p) {
        hr_memzero(p, size);
    }

    return p;
}


hr_pool_cleanup_t *
hr_pool_cleanup_add(hr_pool_t *p, size_t size)
{
    hr_pool_cleanup_t  *c;

    c = hr_palloc(p, sizeof(hr_pool_cleanup_t));
    if (c == NULL) {
        return NULL;
    }

    if (size) {
        c->data = hr_palloc(p, size);
        if (c->data == NULL) {
            return NULL;
        }

    } else {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = p->cleanup;

    p->cleanup = c;
    return c;
}

