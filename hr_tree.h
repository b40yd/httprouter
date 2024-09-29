#ifndef _HR_TREE_H_
#define _HR_TREE_H_

#include "hr_core.h"
#include "hr_string.h"
#include "hr_array.h"

#define ESCAPE_COLON_FALSE  0
#define ESCAPE_COLON_TRUE   1
#define CHAR_COLON          ':'
#define CHAR_STAR           '*'
#define CHAR_SLASH          '/'

typedef enum {
    PUBLIC,
    ROOT,
    PARAM,
    CATCHALL,
} hr_node_type_t;

typedef struct {
    hr_str_t  wildcard;
    int i;
    hr_uint_t valid;
} hr_wildcard_t;

typedef struct hr_node_s {
    hr_str_t           path;
    hr_str_t           indices;
    hr_uint_t          wildchild;
    hr_node_type_t     node_type; 
    uint32_t           priority;
    hr_array_t        *children;
    hr_str_t           full_path;
} hr_node_t;

typedef struct hr_method_tree_s {
    hr_str_t   method;
    hr_node_t *root;
} hr_method_tree_t;

typedef struct hr_router_param_s {
    hr_str_t key;
    hr_str_t value;
} hr_router_param_t;

hr_str_t *get_router_param_by_name(hr_array_t *params);

hr_node_t *hr_create_node(hr_pool_t *pool);
hr_wildcard_t hr_find_wildcard(hr_str_t *path);
uint16_t hr_count_params(hr_str_t *path);
void add_route(hr_pool_t *pool, hr_node_t *n, hr_str_t path);
void hr_rebuild_indices(hr_node_t *n, int pos, int new_pos);
int hr_increment_child_priority(hr_node_t *n, int pos);

void get_value(hr_pool_t *pool, hr_node_t *n, hr_str_t *path, hr_array_t *router_params);

#endif