#ifndef _HR_PALLOC_H_INCLUDED_
#define _HR_PALLOC_H_INCLUDED_

#include "hr_core.h"

#define HR_MAX_ALLOC_FROM_POOL  4095

#define HR_DEFAULT_POOL_SIZE    (16 * 1024)

#define HR_POOL_ALIGNMENT       16
#define HR_MIN_POOL_SIZE                                                     \
    hr_align((sizeof(hr_pool_t) + 2 * sizeof(hr_pool_large_t)),            \
              HR_POOL_ALIGNMENT)

#ifndef HR_ALIGNMENT
#define HR_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#endif

#define hr_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define hr_align_ptr(p, a)                                                   \
    (hr_u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define hr_memzero(buf, n)       (void) memset(buf, 0, n)

typedef struct hr_pool_s            hr_pool_t;

typedef void (*hr_pool_cleanup_pt)(void *data);

typedef struct hr_pool_cleanup_s  hr_pool_cleanup_t;

struct hr_pool_cleanup_s {
    hr_pool_cleanup_pt   handler;
    void                 *data;
    hr_pool_cleanup_t   *next;
};


typedef struct hr_pool_large_s  hr_pool_large_t;

struct hr_pool_large_s {
    hr_pool_large_t     *next;
    void                 *alloc;
};


typedef struct {
    hr_u_char               *last;
    hr_u_char               *end;
    hr_pool_t           *next;
    hr_uint_t            failed;
} hr_pool_data_t;


struct hr_pool_s {
    hr_pool_data_t       d;
    size_t                max;
    hr_pool_t           *current;
    hr_pool_large_t     *large;
    hr_pool_cleanup_t   *cleanup;
};

void *hr_alloc(size_t size);
void *hr_calloc(size_t size);

#define hr_free          free

hr_pool_t *hr_create_pool(size_t size);
void hr_destroy_pool(hr_pool_t *pool);
void hr_reset_pool(hr_pool_t *pool);

void *hr_palloc(hr_pool_t *pool, size_t size);
void *hr_pnalloc(hr_pool_t *pool, size_t size);
void *hr_pcalloc(hr_pool_t *pool, size_t size);
void *hr_pmemalign(hr_pool_t *pool, size_t size, size_t alignment);
hr_int_t hr_pfree(hr_pool_t *pool, void *p);

#endif /* _HR_PALLOC_H_INCLUDED_ */