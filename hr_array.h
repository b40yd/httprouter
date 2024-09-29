#ifndef _HR_ARRAY_H_INCLUDED_
#define _HR_ARRAY_H_INCLUDED_

#include "hr_core.h"
#include "hr_palloc.h"
#include "hr_string.h"

typedef struct {
    void        *elts;
    hr_uint_t   nelts;
    size_t       size;
    hr_uint_t   nalloc;
    hr_pool_t  *pool;
} hr_array_t;


hr_array_t *hr_array_create(hr_pool_t *p, hr_uint_t n, size_t size);
void hr_array_destroy(hr_array_t *a);
void *hr_array_push(hr_array_t *a);
void *hr_array_push_n(hr_array_t *a, hr_uint_t n);


static inline hr_int_t
hr_array_init(hr_array_t *array, hr_pool_t *pool, hr_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = hr_palloc(pool, n * size);
    if (array->elts == NULL) {
        return HR_ERROR;
    }

    return HR_OK;
}


#endif /* _HR_ARRAY_H_INCLUDED_ */