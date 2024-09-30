#include "hr_tree.h"
#include "hr_palloc.h"

hr_node_t *hr_create_node(hr_pool_t *pool) {
    hr_node_t *node = NULL;
    hr_array_t *children = NULL;
    if (pool == NULL) {
        return NULL;
    }
    
    node = hr_palloc(pool, sizeof(hr_node_t));
    if (node == NULL) {
        return NULL;
    }

    children = hr_array_create(pool, 1, sizeof(hr_node_t));
    if (children == NULL) {
        return NULL;
    }

    node->children = children;

    return node; 
}

hr_node_t *hr_node_init(hr_pool_t *pool, hr_node_t *node) {
    hr_array_t *children = NULL;
    if (pool == NULL) {
        return NULL;
    }

    children = hr_array_create(pool, 1, sizeof(hr_node_t));
    if (children == NULL) {
        return NULL;
    }

    node->children = children;

    return node; 
}

hr_wildcard_t hr_find_wildcard(hr_str_t *path) {
    hr_wildcard_t result = {hr_null_string, -1, 0};

    // Find start
    for (int start = 0; start < path->len; start++) {
        uint8_t escape_colon = ESCAPE_COLON_FALSE;
        char c = path->data[start];

        if (escape_colon == ESCAPE_COLON_TRUE) {
            escape_colon = ESCAPE_COLON_FALSE;
            if (c == CHAR_COLON) {
                continue;
            }
        }

        if (c == '\\') {
			escape_colon = ESCAPE_COLON_TRUE;
			continue;
		}

        // A wildcard starts with ':' (param) or '*' (catch-all)
        if (c != CHAR_COLON && c != CHAR_STAR) {
            continue;
        }

        // Find end and check for invalid characters
        int valid = 1;
        for (int end = start + 1; end <= path->len; end++) {
            switch (path->data[end])
            {
            case CHAR_SLASH:
                result.wildcard.len = end - start;
                result.wildcard.data = path->data + start;
                result.i = start;
                result.valid = valid;
                return result;

            case CHAR_COLON:
            case CHAR_STAR:
                valid = 0;
                break;
            }
        }

        result.wildcard.len = path->len - start;
        result.wildcard.data = path->data + start;
        result.i = start;
        result.valid = valid;
        return result;
    }
    
    return result;
}

uint16_t hr_count_params(hr_str_t *path) {
    uint16_t n = 0;
    for (int i = 0; i < path->len; i++) {
        switch (path->data[i]) {
        case CHAR_COLON:
        case CHAR_STAR:
            n++;
        }
    }
    return n;
}

uint16_t hr_count_sections(hr_str_t *path) {
    uint16_t n = 0;
    for (int i = 0; i < path->len; i++) {
        if (path->data[i] == CHAR_SLASH) {
            n++;
        }
    }
    return n;
}

// Build new index char string
void hr_rebuild_indices(hr_node_t *n, int pos, int new_pos) {
    if (n == NULL || n->indices.data == NULL) {
        return;
    }

    int len = n->indices.len;
    if (len <= 0 || pos < 0 || new_pos < 0 || pos >= len || new_pos >= len) {
        return;
    }

    if (new_pos != pos) {

        hr_u_char *str = n->indices.data;
        hr_u_char result[len + 1];
        int i, j = 0;

        for (i = 0; i < new_pos; i++) {
            if (i == pos) {
                continue;
            }
            result[j++] = str[i];
        }

        result[j++] = str[pos];

        for (i = new_pos; i < pos; i++) {
            result[j++] = str[i];
        }

        for (i = pos + 1; i < len; i++) {
            result[j++] = str[i];
        }

        result[j] = '\0';
        
        hr_memcpy(n->indices.data, result, len);
        n->indices.data = result;
        n->indices.len = len;
    }
}

hr_node_t *get_child_node(hr_array_t *children, int pos) {
    if (pos < 0 || pos >= children->nelts) {
        return NULL;
    }

    hr_node_t *elements = (hr_node_t *)children->elts;
    return elements + pos;
}

void swap_node(hr_node_t* a, hr_node_t* b) {
    hr_node_t temp = *a;
    *a = *b;
    *b = temp;
}

int hr_increment_child_priority(hr_node_t *n, int pos) {
    hr_array_t *cs = n->children;
    uint32_t prio = 0;
    int new_pos = 0;
    hr_node_t *elements;
    hr_node_t *el;

    el = get_child_node(cs, pos);
    if (el == NULL) {
        printf("Index out of bounds\n");
        return new_pos;
    }

    el->priority++;
    prio = el->priority;

    new_pos = pos;
    elements = (hr_node_t *)cs->elts;
    for (el = elements + (new_pos - 1); new_pos > 0 && el->priority < prio; new_pos--) {
        swap_node(&((hr_node_t *)cs->elts)[new_pos - 1], &((hr_node_t *)cs->elts)[new_pos]);
    }

    hr_rebuild_indices(n, pos, new_pos);
    
    return new_pos;
}

void insert_child(hr_pool_t *pool, hr_node_t *n, hr_str_t path, hr_str_t full) {
    hr_node_t *child = NULL;
    for (;;) {
        hr_wildcard_t wildcard = hr_find_wildcard(&path);
        int i = wildcard.i;
        if (i < 0) {
            // No wildcard found
            break;
        }

        if (!wildcard.valid) {
            // error
            printf("only one wildcard per path segment is allowed %d\n", i);
            break;
        }

        if (wildcard.wildcard.len < 2) {
            // error
            printf("wildcards must be named with a non-empty name in path");
            break;
        }

        if(n->children->nelts > 0) {
            break;
        }

        if (wildcard.wildcard.data[0] == CHAR_COLON) {
            if (i > 0) {
                n->path.data = hr_pnalloc(pool, i);
                hr_memzero(n->path.data, i);
                if (n->path.data == NULL) {
                    printf("memory allow failed.");
                    break;
                }
                n->path.data = hr_copy(n->path.data, path.data, i);
                n->path.len = i;
                path.data = path.data + i;
                path.len = path.len - i;
            }

            n->wildchild = 1;
            
            child = hr_array_push(n->children);
            child = hr_node_init(pool, child);
            if (child == NULL) {
                break;
            }

            child->node_type = PARAM;
            child->path = wildcard.wildcard;
            child->full_path = full;

            n = child;
            n->priority++;

            if (wildcard.wildcard.len < path.len) {
                path.data = path.data + wildcard.wildcard.len;
                path.len = path.len - wildcard.wildcard.len;

                child = hr_array_push(n->children);
                child = hr_node_init(pool, child);
                if (child == NULL) {
                    break;
                }
                child->priority = 1;
                child->full_path = full;

                n = child;
                continue;
            }

            return ;
        }

        if ((i + wildcard.wildcard.len) !=  path.len) {
            // error
            printf("catch-all routes are only allowed at the end of the path in path");
            break;
        }

        if (n->path.len > 0 && n->path.data[n->path.len-1] == CHAR_SLASH) {
            // error
            printf("catch-all conflicts with existing handle for the path segment root in path");
            break;
        }

        i--;
		if (path.data[i] != CHAR_SLASH) {
			printf("no / before catch-all in path");
		}

        hr_u_char *val = hr_palloc(pool, i);
        if (val == NULL) {
            return;
        }
        hr_memzero(val, i);
        hr_memcpy(val, path.data, i);
        path.data = val;
        path.len = i;

        // First node: catchAll node with empty path
        child = hr_array_push(n->children);
        child = hr_node_init(pool, child);
        if (child == NULL) {
            printf("First node: catchAll node with empty path");
            break;
        }
        child->wildchild = 1;
        child->node_type = CATCHALL;
        child->full_path = full;
        n->indices = (hr_str_t)hr_string("/");

        n = child;
        n->priority++;

        // second node: node holding the variable
        child = hr_array_push(n->children);
        child = hr_node_init(pool, child);
        if (child == NULL) {
            printf("second node: node holding the variable");
            break;
        }

        child->path.data = path.data + i;
        child->path.len = path.len - i;
        child->node_type = CATCHALL;
        child->priority = 1;
        child->full_path = full;
        return;
    }

    // If no wildcard was found, simply insert the path and handle
	n->path = path;
    n->full_path = full;
}

void add_route(hr_pool_t *pool, hr_node_t *n, hr_str_t path) {
    hr_str_t full_path = path;
    hr_uint_t parentFullPathIndex = 0;
    n->priority++;

    if (n->path.len == 0 && n->indices.len == 0) {
        insert_child(pool, n, path, full_path);
        n->node_type = ROOT;
        return;
    }

walk:
    for (;;)
    {
        hr_node_t *child = NULL;
        int i = 0;
        i = hr_str_longest_common_prefix(&path, &n->path);
        if (i < n->path.len) {
            hr_array_t *new_nodes = hr_array_create(pool, 1, sizeof(hr_node_t));
            if (new_nodes == NULL) {
                break;
            }
            hr_node_t *node = hr_array_push(new_nodes);

            node->path.data = n->path.data + i;
            node->path.len =  n->path.len - i;
            node->wildchild = n->wildchild;
            node->node_type = PUBLIC;
            node->indices = n->indices;
            node->children = n->children;
            node->priority = n->priority - 1;
            node->full_path = n->full_path;

            n->children = new_nodes;
            n->indices.data = hr_palloc(pool, 1);
            hr_memzero(n->indices.data, 1);
            n->indices.data = hr_copy(n->indices.data, n->path.data + i, 1);
            n->indices.len = 1;

            n->path.data = hr_pnalloc(pool, i);
            hr_memzero(n->path.data, i);
            if (n->path.data == NULL) {
                printf("memory allow failed.");
                break;
            }
            n->path.data = hr_copy(n->path.data, path.data, i);
            n->path.len = i;

            n->wildchild = 0;

            n->full_path.len = parentFullPathIndex + i;
            n->full_path.data = hr_palloc(pool, n->full_path.len);
            hr_memzero(n->full_path.data, n->full_path.len);
            n->full_path.data = hr_copy(n->full_path.data, full_path.data, n->full_path.len);
            
        }

        if (i < path.len) {
            path.data = path.data + i;
            path.len = path.len - i;

            char c = path.data[0];

            if (n->node_type == PARAM && c == CHAR_SLASH && n->children->nelts== 1) {
                parentFullPathIndex += n->path.len;
                n = get_child_node(n->children, 0);
                n->priority++;
                goto walk;
            }

            for (int i = 0, max = n->indices.len; i < max; i++) {
                if (c == n->indices.data[i]) {
                    parentFullPathIndex += n->path.len;
                    // i = hr_increment_child_priority(pool, n, i);
                    n = get_child_node(n->children, i);
                    goto walk;
                }
            }

            if (c != CHAR_COLON && c != CHAR_STAR && n->node_type != CATCHALL) {
                hr_u_char byte_char[2] = {c, '\0'};
                n->indices = hr_str_concat(pool, n->indices, (hr_str_t)hr_string(byte_char));
                
                child = hr_array_push(n->children);
                child = hr_node_init(pool, child);
                if (child == NULL) {
                    return;
                }
                child->full_path = full_path;
                // hr_increment_child_priority(pool, n, n->indices.len - 1);
                n = child;
            } else if (n->wildchild) {
                n = get_child_node(n->children, n->children->nelts - 1);
                n->priority++;

                if (path.len >= n->path.len && 
                    hr_strncmp(n->path.data, path.data, n->path.len) == 0 &&
                    n->node_type == CATCHALL &&
                    (n->path.len >= path.len || path.data[path.len] == CHAR_SLASH)) {
                    
                    goto walk;
                }

                printf("conflicts with existing wildcard");
                return;
            }

            insert_child(pool, n, path, full_path);
            return;
        }

        n->full_path = full_path;
        return;
        
    }
    

}


typedef struct Param {
    char* Key;
    char* Value;
} Param;

typedef Param* Params;


// UTF-8相关实现


int utf8DecodeRune(char* s, int* size) {
    int rune = 0;
    int bytes = 1;
    if ((*s & 0x80) == 0) {
        rune = *s;
    } else if ((*s & 0xE0) == 0xC0) {
        bytes = 2;
        rune = (*s & 0x1F) << 6;
        s++;
        rune |= (*s & 0x3F);
    } else if ((*s & 0xF0) == 0xE0) {
        bytes = 3;
        rune = (*s & 0x0F) << 12;
        s++;
        rune |= (*s & 0x3F) << 6;
        s++;
        rune |= (*s & 0x3F);
    } else if ((*s & 0xF8) == 0xF0) {
        bytes = 4;
        rune = (*s & 0x07) << 18;
        s++;
        rune |= (*s & 0x3F) << 12;
        s++;
        rune |= (*s & 0x3F) << 6;
        s++;
        rune |= (*s & 0x3F);
    }
    *size = bytes;
    return rune;
}

void utf8EncodeRune(char* buf, int rune) {
    if (rune < 0x80) {
        buf[0] = rune;
    } else if (rune < 0x800) {
        buf[0] = 0xC0 | (rune >> 6);
        buf[1] = 0x80 | (rune & 0x3F);
    } else if (rune < 0x10000) {
        buf[0] = 0xE0 | (rune >> 12);
        buf[1] = 0x80 | ((rune >> 6) & 0x3F);
        buf[2] = 0x80 | (rune & 0x3F);
    } else {
        buf[0] = 0xF0 | (rune >> 18);
        buf[1] = 0x80 | ((rune >> 12) & 0x3F);
        buf[2] = 0x80 | ((rune >> 6) & 0x3F);
        buf[3] = 0x80 | (rune & 0x3F);
    }
}

// shiftNRuneBytes函数
void shiftNRuneBytes(char* rb, int n) {
    switch (n) {
    case 0:
        break;
    case 1:
        rb[0] = rb[1];
        rb[1] = rb[2];
        rb[2] = rb[3];
        rb[3] = 0;
        break;
    case 2:
        rb[0] = rb[2];
        rb[1] = rb[3];
        rb[2] = 0;
        rb[3] = 0;
        break;
    case 3:
        rb[0] = rb[3];
        rb[1] = 0;
        rb[2] = 0;
        rb[3] = 0;
        break;
    default:
        rb[0] = 0;
        rb[1] = 0;
        rb[2] = 0;
        rb[3] = 0;
        break;
    }
}

void get_value(hr_pool_t *pool, hr_node_t *n, hr_str_t *path, hr_array_t *router_params) {
    if (router_params == NULL) {
        return;
    }

walk:
    for (;;) {
        hr_str_t prefix = n->path;
        if (path->len > prefix.len) {
            if (hr_strncmp(path->data, prefix.data, prefix.len) == 0) {
                if (path->len == 0) {
                    break;
                }

                path->data = path->data + prefix.len;
                path->len = path->len - prefix.len;

                if (!n->wildchild) {
                    hr_u_char idxc = path->data[0];
                    for (int i = 0; i < n->indices.len; i++) {
                        if (n->indices.data[i] == idxc) {
                            n = get_child_node(n->children, i);
                            goto walk;
                        }
                    }

                    return;
                }

                n = get_child_node(n->children, 0);
                int value_len = 0, key_len = 0;
                switch (n->node_type)
                {
                case PARAM:
                    
                    while (key_len < n->path.len && n->path.data[key_len] != '/')
                    {
                        key_len++;
                    }

                    while (value_len < path->len && path->data[value_len] != '/')
                    {
                        value_len++;
                    }

                    if (router_params != NULL) {
                        hr_router_param_t *param = hr_array_push(router_params);
                        hr_u_char *key = hr_palloc(pool, key_len);
                        if (key == NULL) {
                            return;
                        }
                        hr_memzero(key, value_len);
                        hr_memcpy(key, n->path.data + 1, key_len - 1);
                        param->key.data = key;
                        param->key.len = key_len - 1;

                        hr_u_char *val = hr_palloc(pool, value_len);
                        if (val == NULL) {
                            return;
                        }
                        hr_memzero(val, value_len);
                        hr_memcpy(val, path->data, value_len);
                        param->value.data = val;
                        param->value.len = value_len;

                    }

                    if (value_len < path->len) {
                        if (n->children->nelts > 0) {
                            path->data = path->data + value_len;
                            path->len = path->len - value_len;
                            n = get_child_node(n->children, 0);
                            goto walk;
                        }

                        return ;
                    }

                    if (n->children->nelts == 1) {
                        n = get_child_node(n->children, 0);
                        return;
                    } 

                    printf(" ------- %s ------- \n", n->full_path.data);
                    return;

                case CATCHALL:
                    if (router_params != NULL) {
                        hr_router_param_t *param = hr_array_push(router_params);

                        param->key.data = n->path.data + 2;
                        param->key.len = n->path.len - 2;
                        param->value = *path;
                    }
                    printf(" ------- %s ------- \n", n->full_path.data);
                    return;
                default:
                    printf(" ------- error ------ \n");
                    break;
                }
            }
        } else if (path->len == prefix.len && hr_strncmp(path->data, prefix.data, path->len) == 0) {
            printf(" ------- %s ------- \n", n->full_path.data);
            for (int i = 0; i < n->indices.len; i++) {
                if (n->indices.data[i] == '/') {
                    n = get_child_node(n->children, i);
                    return;
                }
            }

            return;
        }
        printf(" --------- Not found. --------\n");
        return;
    }
}

