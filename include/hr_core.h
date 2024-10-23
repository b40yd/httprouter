#ifndef _HR_CORE_H_
#define _HR_CORE_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef intptr_t        hr_int_t;
typedef uintptr_t       hr_uint_t;
typedef intptr_t        hr_flag_t;

typedef unsigned char hr_u_char;

#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"

#define  HR_OK          0
#define  HR_ERROR      -1
#define  HR_DECLINED   -2
#define  HR_ABORT      -3

#define hr_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define hr_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

#endif