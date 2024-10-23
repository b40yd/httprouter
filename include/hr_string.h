#ifndef _HR_STRING_H_
#define _HR_STRING_H_

#include "hr_core.h"
#include "hr_palloc.h"

typedef struct {
    size_t      len;
    hr_u_char  *data;
} hr_str_t;


typedef struct {
    hr_str_t   key;
    hr_str_t   value;
} hr_keyval_t;



#define hr_string(str)     { sizeof(str) - 1, (hr_u_char *) str }
#define hr_null_string     { 0, NULL }
#define hr_str_set(str, text)                                               \
    (str)->len = sizeof(text) - 1; (str)->data = (hr_u_char *) text
#define hr_str_null(str)   (str)->len = 0; (str)->data = NULL


#define hr_tolower(c)      (hr_u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define hr_toupper(c)      (hr_u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

void hr_strlow(hr_u_char *dst, hr_u_char *src, size_t n);
#define hr_strncmp(s1, s2, n)  strncmp((const char *) s1, (const char *) s2, n)

#define hr_strcmp(s1, s2)  strcmp((const char *) s1, (const char *) s2)


#define hr_strstr(s1, s2)  strstr((const char *) s1, (const char *) s2)
#define hr_strlen(s)       strlen((const char *) s)

size_t hr_strnlen(hr_u_char *p, size_t n);

#define hr_strchr(s1, c)   strchr((const char *) s1, (int) c)

static inline hr_u_char *
hr_strlchr(hr_u_char *p, hr_u_char *last, hr_u_char c)
{
    while (p < last) {

        if (*p == c) {
            return p;
        }

        p++;
    }

    return NULL;
}


hr_str_t hr_str_concat(hr_pool_t *pool, hr_str_t str1, hr_str_t str2);

hr_u_char *hr_cpystrn(hr_u_char *dst, hr_u_char *src, size_t n);

hr_int_t hr_strcasecmp(hr_u_char *s1, hr_u_char *s2);
hr_int_t hr_strncasecmp(hr_u_char *s1, hr_u_char *s2, size_t n);

hr_u_char *hr_strnstr(hr_u_char *s1, char *s2, size_t n);

hr_u_char *hr_strstrn(hr_u_char *s1, char *s2, size_t n);
hr_u_char *hr_strcasestrn(hr_u_char *s1, char *s2, size_t n);
hr_u_char *hr_strlcasestrn(hr_u_char *s1, hr_u_char *last, hr_u_char *s2, size_t n);

size_t hr_str_longest_common_prefix(hr_str_t *a, hr_str_t *b);

hr_u_char *hr_hex_dump(hr_u_char *dst, hr_u_char *src, size_t len);


#define hr_base64_encoded_length(len)  (((len + 2) / 3) * 4)
#define hr_base64_decoded_length(len)  (((len + 3) / 4) * 3)

void hr_encode_base64(hr_str_t *dst, hr_str_t *src);
void hr_encode_base64url(hr_str_t *dst, hr_str_t *src);
hr_int_t hr_decode_base64(hr_str_t *dst, hr_str_t *src);
hr_int_t hr_decode_base64url(hr_str_t *dst, hr_str_t *src);

uint32_t hr_utf8_rune_start(const char c);
uint32_t hr_utf8_decode(hr_u_char **p, size_t n);
size_t hr_utf8_length(hr_u_char *p, size_t n);
hr_u_char *hr_utf8_cpystrn(hr_u_char *dst, hr_u_char *src, size_t n, size_t len);


#define hr_ESCAPE_URI            0
#define hr_ESCAPE_ARGS           1
#define hr_ESCAPE_URI_COMPONENT  2
#define hr_ESCAPE_HTML           3
#define hr_ESCAPE_REFRESH        4
#define hr_ESCAPE_MEMCACHED      5
#define hr_ESCAPE_MAIL_AUTH      6

#define hr_UNESCAPE_URI       1
#define hr_UNESCAPE_REDIRECT  2

uintptr_t hr_escape_uri(hr_u_char *dst, hr_u_char *src, size_t size,
    hr_uint_t type);
void hr_unescape_uri(hr_u_char **dst, hr_u_char **src, size_t size, hr_uint_t type);
uintptr_t hr_escape_html(hr_u_char *dst, hr_u_char *src, size_t size);
uintptr_t hr_escape_json(hr_u_char *dst, hr_u_char *src, size_t size);

/*
 * gcc3, msvc, and icc7 compile memcpy() to the inline "rep movs".
 * gcc3 compiles memcpy(d, s, 4) to the inline "mov"es.
 * icc8 compile memcpy(d, s, 4) to the inline "mov"es or XMM moves.
 */
#define hr_memcpy(dst, src, n)   (void) memcpy(dst, src, n)
#define hr_cpymem(dst, src, n)   (((hr_u_char *) memcpy(dst, src, n)) + (n))

static inline hr_u_char *
hr_copy(hr_u_char *dst, hr_u_char *src, size_t len)
{
    int i = 0;
    while (len) {
        *(dst+i) = *(src+i);
        len--;
        i++;
    }

    return dst;
}

#endif