#include "hr_core.h"
#include "hr_list.h"
#include "hr_palloc.h"

hr_list_t *
hr_list_create(hr_pool_t *pool, hr_uint_t n, size_t size)
{
    hr_list_t  *list;

    list = hr_palloc(pool, sizeof(hr_list_t));
    if (list == NULL) {
        return NULL;
    }

    if (hr_list_init(list, pool, n, size) != HR_OK) {
        return NULL;
    }

    return list;
}


void *
hr_list_push(hr_list_t *l)
{
    void             *elt;
    hr_list_part_t  *last;

    last = l->last;

    if (last->nelts == l->nalloc) {

        /* the last part is full, allocate a new list part */

        last = hr_palloc(l->pool, sizeof(hr_list_part_t));
        if (last == NULL) {
            return NULL;
        }

        last->elts = hr_palloc(l->pool, l->nalloc * l->size);
        if (last->elts == NULL) {
            return NULL;
        }

        last->nelts = 0;
        last->next = NULL;

        l->last->next = last;
        l->last = last;
    }

    elt = (char *) last->elts + l->size * last->nelts;
    last->nelts++;

    return elt;
}