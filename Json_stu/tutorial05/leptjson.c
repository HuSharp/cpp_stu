/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 21:39:51
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-15 23:23:13
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
        while (c->top + len >= c->size)
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

/* 解析 4 位十六进数字，存储为码点 u 
   成功时返回解析后的文本指针，失败返回 NULL */
static const char* lept_parse_string_hex4(const char* pos, unsigned* u) {
    size_t cnt;
    *u = 0;
    for (cnt = 0; cnt < 4; cnt++) {
        char ch = *pos++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9')         *u |= ch - '0';
        else if (ch >= 'a' && ch <= 'f')    *u |= ch - ('a' - 10);
        else if (ch >= 'A' && ch <= 'F')    *u |= ch - ('A' - 10);
        else return NULL;
    }
    return pos;
}

/*
U+0000 至 U+FFFF 这组 Unicode 字符称为基本多文种平面（basic multilingual plane, BMP），
BMP 以外还有另外 16 个平面。
那么对于 BMP 以外的字符，JSON 会使用代理对（surrogate pair）表示 \uXXXX\uYYYY。
BMP 中，保留了 2048 个代理码点。
如果第一个码点是 U+D800 至 U+DBFF，我们便知道它的代码对的高代理项（high surrogate），
之后应该伴随一个 U+DC00 至 U+DFFF 的低代理项（low surrogate）。 
代理对 (H, L) 变换公式： codepoint = 0x10000 + (H − 0xD800) × 0x400 + (L − 0xDC00) */
static int lept_parse_string_check_surrogate_pair(const char* pos, unsigned* u1, unsigned* u2) {
    if (*u1 >= 0xD800 && *u1 <= 0xDBFF) {
        if (*pos++ != '\\')
            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
        if (*pos++ != 'u')
            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
        if (!(pos = lept_parse_string_hex4(pos, u2)))
            return LEPT_PARSE_INVALID_UNICODE_HEX;
        if (*u2 < 0xDC00 || *u2 > 0xDFFF)
            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
        *u1 = 0x10000 + (((*u1 - 0xD800) << 10) | (*u2 - 0xDC00));
        return LEPT_PARSE_INVALID_SURROGATE_OK;
    }
    return LEPT_PARSE_OK;
}

/* 码点编码成 UTF-8，写进缓冲区 */
static void lept_parse_string_encode_utf8(lept_context* c, unsigned u) {
    if (u <= 0x7F) {
        PUTC(c, u & 0x7F);                      /* 0x7F = 01111111 */
    }
    else if (u <= 0x7FF) {
        PUTC(c, 0xC0 | ((u >> 6) & 0x1F));      /* 0xC0 = 11000000 */
        PUTC(c, 0x80 | (u & 0x3F));             /* 0x3F = 00111111 */
    }
    else if (u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0xF));      /* 0xE0 = 11100000 */
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));      /* 0x80 = 10000000 */
        PUTC(c, 0x80 | (u & 0x3F));             /* 0x3F = 00111111 */
    }
    else if (u <= 0x10FFFF) {
        PUTC(c, 0xF0 | ((u >> 18) & 0x7));      /* 0xF0 = 11110000 */
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));     /* 0x07 = 00000111 */
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));      /* 0x80 = 10000000 */
        PUTC(c, 0x80 | (u & 0x3F));             /* 0x3F = 00111111 */
    }
}

#define STRING_ERROR(error)    do {c->top = head; return error;} while(0)

static int lept_parse_string(lept_context* c, lept_value* v) {
    size_t head = c->top, len;
    const char* pos;
    unsigned u1, u2; /* 存储码点 */
    int check_surrogate_pair;
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
            STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
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
            case 'u': 
                if (!(pos = lept_parse_string_hex4(pos, &u1))) 
                    STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                /* 进行代理项对的检测 */
                check_surrogate_pair = lept_parse_string_check_surrogate_pair(pos, &u1, &u2);
                if (LEPT_PARSE_OK == check_surrogate_pair || LEPT_PARSE_INVALID_SURROGATE_OK == check_surrogate_pair) {
                    if(LEPT_PARSE_INVALID_SURROGATE_OK == check_surrogate_pair) pos += 6;   /* eg: \uDD1E 6 个字符*/
                } else  STRING_ERROR(check_surrogate_pair);
                /* 进行编码 */
                lept_parse_string_encode_utf8(c, u1);
                break;
            default:
                STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
            }
            break;
        default:
            /* unescaped = %x20-21 / %x23-5B / %x5D-10FFFF 
                当中空缺的 %x22 是双引号，%x5C 是反斜线，都已经处理。
                所以不合法的字符是 %x00 至 %x1F。*/
            if ((unsigned char)ch < 0x20) {
                STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
            }
            PUTC(c, ch);  /* set character */
        }
    }
}

static int lept_parse_value(lept_context* c, lept_value* v); /* 前向声明 */

/* 
在循环中建立一个临时值（lept_value e），
然后调用 lept_parse_value() 去把元素解析至这个临时值，完成后把临时值压栈。
当遇到 ]，把栈内的元素弹出，分配内存，生成数组值。
*/
static int lept_parse_array(lept_context* c, lept_value* v) {
    int ret;
    size_t size=0, i;
    EXPECT(c, '[');
    lept_parse_whitespace(c);
    if (*c->json == ']') {
        c->json++;
        v->type = LEPT_ARRAY;
        v->u.arr.size = 0;
        v->u.arr.e = NULL;
        return LEPT_PARSE_OK;
    }

    while (1) {
        lept_value e;
        lept_init(&e);
        if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK) {
            break;
        }
        size++; /* 记录压栈个数 */
        memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
        lept_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            lept_parse_whitespace(c);
        } else if (*c->json == ']') {
            c->json++;
            v->type = LEPT_ARRAY;
            v->u.arr.size = size;
            size *= sizeof(lept_value);
            memcpy(v->u.arr.e = (lept_value*)malloc(size), lept_context_pop(c, size), size);
            return LEPT_PARSE_OK;
        } else {
            ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
     /* Pop and free values on the stack */
    for (i = 0; i < size; i++) {
        lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
    }
    
    return ret;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':   return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':   return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':   return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '\"':  return lept_parse_string(c, v);
        case '[':   return lept_parse_array(c, v);
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
    size_t i;
    assert(v != NULL);
    switch (v->type){
    case LEPT_STRING:
        free(v->u.str.s);
        break;
    case LEPT_ARRAY:
        for (i = 0; i < v->u.arr.size; i++) {
            lept_free(&v->u.arr.e[i]);
        }
        free(v->u.arr.e);
        break;
    default:
        break;
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

void lept_set_number(lept_value* v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

size_t lept_get_array_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.arr.size;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    assert(index < v->u.arr.size);
    return &v->u.arr.e[index];
}