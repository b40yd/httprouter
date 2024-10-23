#include "hr_array.h"

hr_array_t *
hr_array_create(hr_pool_t *p, hr_uint_t n, size_t size)
{
    hr_array_t *a;

    a = hr_palloc(p, sizeof(hr_array_t));
    if (a == NULL) {
        return NULL;
    }

    if (hr_array_init(a, p, n, size) != HR_OK) {
        return NULL;
    }

    return a;
}


void
hr_array_destroy(hr_array_t *a)
{
    hr_pool_t  *p;

    p = a->pool;

    if ((hr_u_char *) a->elts + a->size * a->nalloc == p->d.last) {
        p->d.last -= a->size * a->nalloc;
    }

    if ((hr_u_char *) a + sizeof(hr_array_t) == p->d.last) {
        p->d.last = (hr_u_char *) a;
    }
}


void *
hr_array_push(hr_array_t *a)
{
    void        *elt, *new;
    size_t       size;
    hr_pool_t  *p;

    if (a->nelts == a->nalloc) {

        /* the array is full */

        size = a->size * a->nalloc;

        p = a->pool;

        if ((hr_u_char *) a->elts + size == p->d.last
            && p->d.last + a->size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->size;
            a->nalloc++;

        } else {
            /* allocate a new array */

            new = hr_palloc(p, 2 * size);
            if (new == NULL) {
                return NULL;
            }

            hr_memcpy(new, a->elts, size);
            a->elts = new;
            a->nalloc *= 2;
        }
    }

    elt = (hr_u_char *) a->elts + a->size * a->nelts;
    a->nelts++;

    return elt;
}


void *
hr_array_push_n(hr_array_t *a, hr_uint_t n)
{
    void        *elt, *new;
    size_t       size;
    hr_uint_t   nalloc;
    hr_pool_t  *p;

    size = n * a->size;

    if (a->nelts + n > a->nalloc) {

        /* the array is full */

        p = a->pool;

        if ((hr_u_char *) a->elts + a->size * a->nalloc == p->d.last
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;
            a->nalloc += n;

        } else {
            /* allocate a new array */

            nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);

            new = hr_palloc(p, nalloc * a->size);
            if (new == NULL) {
                return NULL;
            }

            hr_memcpy(new, a->elts, a->nelts * a->size);
            a->elts = new;
            a->nalloc = nalloc;
        }
    }

    elt = (hr_u_char *) a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}