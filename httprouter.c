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

    hr_rebuild_indices(pool, demo, 3, 2);

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


    hr_node_t *tmp = hr_palloc(pool, sizeof(hr_node_t));
    tmp->children = hr_array_create(pool, 2, sizeof(hr_node_t));
    tmp->path = (hr_str_t)hr_string("/");
    tmp->indices.data = (hr_u_char *)hr_palloc(pool, 3); 
    tmp->indices.len = 3;  
    tmp->indices.data[0] = 'd';
    tmp->indices.data[1] = 'c';
    tmp->indices.data[2] = 'a';
    hr_u_char d[3] = {'d', 'c','a'};
    hr_node_t *tmp2 ;
    for (int i = 0; i < 3; i++) {
        tmp2 = hr_array_push(tmp->children);
        // hr_node_t *tmp2 = hr_palloc(pool, sizeof(hr_node_t));
        hr_u_char *data = hr_palloc(pool, 3);
        data[0] = '/';
        data[1] = d[i];
        data[2] = '\0';
        tmp2->path = (hr_str_t)hr_string(data);
        tmp2->path.len = 2;


        hr_u_char *data2 = hr_palloc(pool, 2);
        data2[0] = d[i];
        data2[1] = '\0';
        tmp2->indices = (hr_str_t)hr_string(data2);
        tmp2->indices.len = 1;
        tmp2->priority += i;
        printf("....... %s (%lu): %s(%lu)  %d.....\n", tmp2->path.data, tmp2->path.len, tmp2->indices.data, tmp2->indices.len, tmp2->priority);

        hr_increment_child_priority(pool, tmp, tmp2->priority);
    }

    // 

    printf("================= sorted ================\n");


    printf(" path: %s, indices: %s  children_len: %d \n", tmp->path.data, tmp->indices.data, tmp->children->nelts);

    for (int pos = 0; pos < tmp->children->nelts; pos++) {
        hr_node_t *elems = tmp->children->elts;
        printf("..... %s(%lu) ....\n", (elems+pos)->path.data, (elems+pos)->path.len);
    }


    // =========== destroy pool ===================
    hr_destroy_pool(pool);
    return 0;
}