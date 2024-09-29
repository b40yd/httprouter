#ifndef _HR_LIST_H_INCLUDED_
#define _HR_LIST_H_INCLUDED_


#include "hr_core.h"
#include "hr_palloc.h"

typedef struct hr_list_part_s  hr_list_part_t;

struct hr_list_part_s {
    void             *elts;
    hr_uint_t        nelts;
    hr_list_part_t  *next;
};


typedef struct {
    hr_list_part_t  *last;
    hr_list_part_t   part;
    size_t            size;
    hr_uint_t        nalloc;
    hr_pool_t       *pool;
} hr_list_t;


hr_list_t *hr_list_create(hr_pool_t *pool, hr_uint_t n, size_t size);

static inline hr_int_t
hr_list_init(hr_list_t *list, hr_pool_t *pool, hr_uint_t n, size_t size)
{
    list->part.elts = hr_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return HR_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return HR_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *hr_list_push(hr_list_t *list);


#endif /* _HR_LIST_H_INCLUDED_ */