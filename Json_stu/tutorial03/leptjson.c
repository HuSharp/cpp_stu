/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 21:39:51
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-12 21:54:17
 * @@Email: 8211180515@csu.edu.cn
 */
#include "leptjson.h"
#include <assert.h>     /* assert() */
#include <errno.h>      /* errno, ERANGE */
#include <math.h>       /* HUGE_VAL */
#include <stdlib.h>     /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>     /* memcpy() */

#define EXPECT(c, ch)   do { assert(*c->json == (ch)); c->json++; } while(0)


typedef struct {
    const char *json;
    char* stack;            /* 把解析的结果先储存在一个临时的缓冲区 */
    size_t size, top;       /* top 为栈顶位置， 由于还会 realloc 进行扩展， 因此不采用指针 */
}lept_context;


#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

static void* lept_context_push(lept_context* c, size_t len) {
    void* ret;
    assert(len > 0);
    if (c->top + len >= c->size) {
        if (c->size == 0) 
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (c->top)
            c->size += c->size >> 1; /* c->size*1.5 */ 
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += len;
    return ret;
}

/* return store value c->stack[] */
static void* lept_context_pop(lept_context* c, size_t len) {
    assert(c->top >= len);
    return c->stack + (c->top -= len);
}

/* ws = *(%x20 / %x09 / %x0A / %x0D) */
static void lept_parse_whitespace(lept_context* c) {
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type expect_type) {
    size_t idx;
    EXPECT(c, literal[0]);
    for (idx = 0; literal[idx+1]; idx++) {
        if (c->json[idx] != literal[idx+1]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += idx;
    v->type = expect_type;
    return LEPT_PARSE_OK;
}

double lept_get_number_val(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

/* 把十进制的数字转换成二进制的 double */
#define ISDIGIT(character)      ((character) >= '0' && (character) <= '9' )
#define ISDIGIT1TO9(character)  ((character) >= '1' && (character) <= '9' ) 

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* pos = c->json;/* 表示当前的解析字符位置 */

    /* number = [ "-" ] int [ frac ] [ exp ] */
    if (*pos == '-')    pos++;
    /* int = "0" / digit1-9 *digit */
    if (*pos == '0')    pos++;
    else {
        if (!ISDIGIT1TO9(*pos))     return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*pos))   pos++;
    }
    /* frac = "." 1*digit */
    if (*pos == '.') {
        pos++;
        if (!ISDIGIT(*pos))     return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*pos))   pos++;
    }
    /* exp = ("e" / "E") ["-" / "+"] 1*digit */
    if (*pos == 'e' || *pos == 'E' ) {
        pos++;
        if (*pos == '-' || *pos == '+' ) pos++;
        if (!ISDIGIT(*pos))     return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*pos))   pos++;
    }

    errno = 0;
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }

    c->json = pos;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

#define PUTC(c, ch)     do {*(char*)lept_context_push(c, sizeof(char)) = (ch);} while(0)

static int lept_parse_string(lept_context* c, lept_value* v) {
    size_t head = c->top, len;
    const char* pos;
    EXPECT(c, '\"');

    pos = c->json;
    while (1) {
        char ch = *pos++;
        switch (ch) {
        case '\"':
            len = c->top - head;
            lept_set_string(v, (const char*)lept_context_pop(c, len), len);
            c->json = pos;
            return LEPT_PARSE_OK;
        case '\0':
            c->top = head;
            return LEPT_PARSE_MISS_QUOTATION_MARK;
        case '\\':
            switch (*pos++) {
            case '"':   PUTC(c, '"');  break;
            case '\\':  PUTC(c, '\\');  break;
            case '/':   PUTC(c, '/');   break;
            case 'b':   PUTC(c, '\b');  break;
            case 'f':   PUTC(c, '\f');  break;
            case 'n':   PUTC(c, '\n');  break;
            case 'r':   PUTC(c, '\r');  break;
            case 't':   PUTC(c, '\t');  break;
            default:
                c->top = head;
                return LEPT_PARSE_INVALID_STRING_ESCAPE;
            }
            break;
        default:
            /* unescaped = %x20-21 / %x23-5B / %x5D-10FFFF 
                当中空缺的 %x22 是双引号，%x5C 是反斜线，都已经处理。
                所以不合法的字符是 %x00 至 %x1F。*/
            if ((unsigned char)ch < 0x20) {
                c->top = head;
                return LEPT_PARSE_INVALID_STRING_CHAR;
            }
            PUTC(c, ch);  /* set character */
        }
    }
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':   return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':   return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':   return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '\"':   return lept_parse_string(c, v);
        default:    return lept_parse_number(c, v);
        case '\0':  return LEPT_PARSE_EXPECT_VALUE;
    }
}

/* parse JSON */
/* 提示：这里应该是 JSON-text = ws value ws */
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v !=  NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v);

    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

void lept_init(lept_value* v) {
    assert(v != NULL);
    v->type = LEPT_NULL;
}

/* const int *ptr 表示指向常量的指针 */
lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

void lept_set_type(lept_value* v, lept_type t) {
    assert(v != NULL);
    v->type = t;
}

void lept_free(lept_value* v) {
    assert(v != NULL);
    if (v->type == LEPT_STRING) {
        free(v->u.str.s);
    }
    v->type = LEPT_NULL;
}

const char* lept_get_string(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.str.s;
}
int lept_get_string_length(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.str.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));

    lept_free(v);
    v->u.str.s = (char*)malloc(len + 1);
    memcpy(v->u.str.s, s, len);

    v->u.str.s[len] = '\0';
    v->u.str.len = len;
    v->type = LEPT_STRING;
}

int lept_get_boolean(const lept_value* v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}


double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

void lept_set_number(lept_value* v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}