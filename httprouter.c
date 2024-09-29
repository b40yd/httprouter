#include "hr_tree.h"
#include "hr_palloc.h"
#include "hr_string.h"

int main(int argc, char const *argv[])
{
    hr_str_t hu = hr_string("/hello");
    hr_wildcard_t wildcard_result;
    
    hr_str_t uri1 = hr_string("/hello");
    hr_str_t uri2 = hr_string("/hello/:apple");
    hr_str_t uri3 = hr_string("/hello/:a/:b");

    size_t index;
    index = hr_str_longest_common_prefix(&uri1, &uri2);
    printf(" ====== (%s:%zu) (%s:%zu) longest common prefix: %zu ======\n", uri1.data, uri1.len, uri2.data, uri2.len, index);

    index = hr_str_longest_common_prefix(&uri2, &uri3);
    printf(" ====== (%s:%zu) (%s:%zu) longest common prefix: %zu ======\n", uri2.data, uri2.len, uri3.data, uri3.len, index);

    wildcard_result = hr_find_wildcard(&uri2);
    printf("======== (%s:%zu) params name: (%s:%zu), in pos (%d), is params: %lu =======\n", uri2.data, uri2.len, 
        wildcard_result.wildcard.data,
        wildcard_result.wildcard.len,
        wildcard_result.i, 
        wildcard_result.valid);
    
    uint16_t i = hr_count_params(&uri2);
    uint16_t i2 = hr_count_params(&uri3);
    printf("------- (%s:%zu) (%s:%zu) url params: (%hu) (%hu) -------\n", uri2.data, uri2.len, uri3.data, uri3.len, i, i2);
    
    int stat;
    stat =  hr_utf8_rune_start('a');
    printf("...... 'a' is utf8 = %d ......\n", stat);

    // ======================= create pool =======================
    hr_pool_t *pool = hr_create_pool(1024);

    // ========================= test rebuild indices ============
    hr_node_t *demo = hr_palloc(pool, sizeof(hr_node_t));
    demo->indices.data = (hr_u_char *)hr_palloc(pool, 6);
    demo->indices.data = hr_copy(demo->indices.data, (hr_u_char *)"123456", 6);
    demo->indices.len = 6;

    hr_rebuild_indices(demo, 3, 2);

    printf("______ %s: %lu _____\n", demo->indices.data, demo->indices.len);
   
    // ========================== test router ====================

    
    printf("....pool:. %ld ...\n", pool->max);
    hr_node_t *node = hr_create_node(pool);

    
    wildcard_result = hr_find_wildcard(&hu);
    printf("----(%s:%zu)  %s %zu %d --------\n", hu.data, hu.len, wildcard_result.wildcard.data, wildcard_result.wildcard.len, wildcard_result.i);
    printf("====test==== (%s:%zu) params name: (%s:%zu), in pos (%d), is params: %lu =======\n", hu.data, hu.len, 
        wildcard_result.wildcard.data,
        wildcard_result.wildcard.len,
        wildcard_result.i, 
        wildcard_result.valid);

    add_route(pool, node, (hr_str_t)hr_string("/hello"));
    add_route(pool, node, (hr_str_t)hr_string("/hello/a/:name/1"));
    add_route(pool, node, (hr_str_t)hr_string("/hello/test/:demo"));
    add_route(pool, node, (hr_str_t)hr_string("/1/hello/test/:demo"));
    add_route(pool, node, (hr_str_t)hr_string("/2/hello/test/:demo"));
    add_route(pool, node, (hr_str_t)hr_string("/2/hello1/:test/:demo"));

    add_route(pool, node, (hr_str_t)hr_string("/www.a.com/test/:hello"));
    add_route(pool, node, (hr_str_t)hr_string("/www.a.com/2/hello/:test1/:hello1"));

    hr_array_t *router_params = hr_array_create(pool, 10, sizeof(hr_router_param_t));
    get_value(pool, node, &(hr_str_t)hr_string("/hello"), router_params);

    get_value(pool, node, &(hr_str_t)hr_string("/hello/a/1/1"), router_params);
    get_value(pool, node, &(hr_str_t)hr_string("/hello/a/2/1"), router_params);

    get_value(pool, node, &(hr_str_t)hr_string("/hello/test/hello"), router_params);

    get_value(pool, node, &(hr_str_t)hr_string("/1/hello/test/hello"), router_params);
    get_value(pool, node, &(hr_str_t)hr_string("/2/hello/test/hello"), router_params);

    get_value(pool, node, &(hr_str_t)hr_string("/2/hello/test/bao"), router_params);
    get_value(pool, node, &(hr_str_t)hr_string("/2/hello/test/bao"), router_params);

    get_value(pool, node, &(hr_str_t)hr_string("/www.a.com/test/:hell1o"), router_params);
    get_value(pool, node, &(hr_str_t)hr_string("/www.a.com/2/hello/2/4"), router_params);

    for (int pos = 0; pos < router_params->nelts; pos++) {
        hr_router_param_t *elements = router_params->elts;
        printf("..... %s(%lu): %s ....\n", (elements+pos)->key.data, (elements+pos)->key.len, (elements+pos)->value.data);
    }

    // =========== destroy pool ===================
    hr_destroy_pool(pool);
    return 0;
}