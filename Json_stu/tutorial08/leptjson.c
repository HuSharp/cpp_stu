/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 21:39:51
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-19 23:39:27
 * @@Email: 8211180515@csu.edu.cn
 */
#include "leptjson.h"
#include <assert.h>     /* assert() */
#include <errno.h>      /* errno, ERANGE */
#include <math.h>       /* HUGE_VAL */
#include <stdlib.h>     /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>     /* memcpy() */
#include <stdio.h>      /* sprintf() */

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

/* 解析 4 位十六进制的数字，存储为码点 u 
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

/* 解析 JSON 字符串，把结果写入 str 和 len */
/* str 指向 c->stack 中的元素，需要在 c->stack  */
static int lept_parse_string_raw(lept_context* c, char** str, size_t* len) {
    size_t head = c->top;
    const char* pos;
    unsigned u1, u2; /* 存储码点 */
    int check_surrogate_pair;
    EXPECT(c, '\"');

    pos = c->json;
    while (1) {
        char ch = *pos++;
        switch (ch) {
        case '\"':
            *len = c->top - head;
            *str = lept_context_pop(c, *len);
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

static int lept_parse_string(lept_context* c, lept_value* v) {
    int ret;
    char* s;
    size_t len;
    if ((ret = lept_parse_string_raw(c, &s, &len)) == LEPT_PARSE_OK) {
        lept_set_string(v, s, len); /* 对于 object 也可以把结果复制至 lept_member 的 k 和 klen 字段。 */
    }
    return ret;
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
        lept_set_array(v, 0);
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
            lept_set_array(v, size);
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
static int lept_parse_object(lept_context* c, lept_value* v) {
    size_t i, size;
    lept_member m;
    int ret;
    EXPECT(c, '{');
    lept_parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = LEPT_OBJECT;
        v->u.obj.m = 0;
        v->u.obj.size = 0;
        return LEPT_PARSE_OK;
    }
    m.key = NULL;
    size = 0;
    for (;;) {
        char* str;
        lept_init(&m.val);
        /* parse key */
        if (*c->json != '"') {
            ret = LEPT_PARSE_MISS_KEY;
            break;
        }
        if ((ret = lept_parse_string_raw(c, &str, &m.klen)) != LEPT_PARSE_OK)
            break;
        memcpy(m.key = (char*)malloc(m.klen + 1), str, m.klen);
        m.key[m.klen] = '\0';
        /* parse ws colon ws */
        lept_parse_whitespace(c);
        if (*c->json != ':') {
            ret = LEPT_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        lept_parse_whitespace(c);
        /* parse value */
        if ((ret = lept_parse_value(c, &m.val)) != LEPT_PARSE_OK)
            break;
        memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
        size++;
        m.key = NULL; /* ownership is transferred to member on stack */
        /* parse ws [comma | right-curly-brace] ws */
        lept_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            lept_parse_whitespace(c);
        }
        else if (*c->json == '}') {
            size_t s = sizeof(lept_member) * size;
            c->json++;
            v->type = LEPT_OBJECT;
            v->u.obj.size = size;
            memcpy(v->u.obj.m = (lept_member*)malloc(s), lept_context_pop(c, s), s);
            return LEPT_PARSE_OK;
        }
        else {
            ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    /* Pop and free members on the stack */
    free(m.key);
    for (i = 0; i < size; i++) {
        lept_member* m = (lept_member*)lept_context_pop(c, sizeof(lept_member));
        free(m->key);
        lept_free(&m->val);
    }
    v->type = LEPT_NULL;
    return ret;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':   return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':   return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':   return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '\"':  return lept_parse_string(c, v);
        case '[':   return lept_parse_array(c, v);
        case '{':   return lept_parse_object(c, v);
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
    case LEPT_OBJECT:
        for (i = 0; i < v->u.obj.size; i++) {
            free(v->u.obj.m[i].key);
            lept_free(&v->u.obj.m[i].val);
        }
        free(v->u.obj.m);
        break;
    default:
        break;
    }
    v->type = LEPT_NULL;
}

/*********************** JSON generator  **********************************************************/
#ifndef LET_PARSE_STRINGFY_INIT_SIZE
#define LET_PARSE_STRINGFY_INIT_SIZE 256
#endif

#define PUTS(c, s, len) memcpy(lept_context_push(c, len), s, len)

/* 需要考虑转义字符 \uXXXX 存储为
eg: "\"\\u0024\""  -> "\x24" */
static void lept_stringify_string(lept_context* c, const char* s, size_t len) {
    size_t i = 0;
    assert(s != NULL);
    PUTC(c, '"');
    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)s[i]; /* sprintf里用有符号char按%x输出大于127的值会是4字节而不是1个字节 */
        switch (ch) {
        case '\"':  PUTS(c, "\\\"", 2);  break;
        case '\\':  PUTS(c, "\\\\", 2);  break;
        case '\b':  PUTS(c, "\\b",  2);  break;
        case '\f':  PUTS(c, "\\f",  2);  break;
        case '\n':  PUTS(c, "\\n",  2);  break;
        case '\t':  PUTS(c, "\\t",  2);  break;
        case '\r':  PUTS(c, "\\r",  2);  break;
        default:
        /* 其他少于 0x20 的字符需要转义为 \u00xx 形式
        对于　unicode：不需处理，因为字符串本身是UTF-8，输出的JSON也是。只有 < 32 的才必须转义。*/
        if (ch < 0x20) {
            char buf[7];
            sprintf(buf, "\\u%04X", ch);
            PUTS(c, buf, 6);
        } else PUTC(c, s[i]);
        }
    }
    PUTC(c, '"');
}

static void lept_stringify_value(lept_context* c, const lept_value* v) {
    size_t i;
    switch (v->type) {
    case LEPT_NULL:         PUTS(c, "null", 4); break;
    case LEPT_FALSE:        PUTS(c, "false", 5);break;
    case LEPT_TRUE:         PUTS(c, "true", 4); break;
    case LEPT_NUMBER:{ /* "%.17g" 足够把 double 转换成可还原的文本。 */
        char* buffer = lept_context_push(c, 64);
        int length = sprintf(buffer, "%.17g", v->u.n);
        c->top -= (64 - length);
        break;
    }
    case LEPT_STRING:       lept_stringify_string(c, v->u.str.s, v->u.str.len);  break;
    case LEPT_ARRAY:{
        PUTC(c, '[');
        for (i = 0; i < v->u.arr.size; i++) {
            if (i > 0) PUTC(c, ',');    /* 在第一个对象之后加入　*/
            lept_stringify_value(c, &v->u.arr.e[i]);
        }
        PUTC(c, ']');
        break;
    }
    case LEPT_OBJECT:{
        PUTC(c, '{');
        for (i = 0; i < v->u.obj.size; i++) {
            if (i > 0) PUTC(c, ',');    /* 在第一个对象之后加入　*/
            lept_member cur_mem = v->u.obj.m[i];
            lept_stringify_string(c, cur_mem.key, cur_mem.klen);
            PUTC(c, ':');
            lept_stringify_value(c, &cur_mem.val);
        }
        PUTC(c, '}');
        break;
    }
    default:                assert(0 && "invalid type");
    }
}

/* 把树形数据结构转换成 JSON 文本。 且继续使用 stack 作为缓冲区。 */
char* lept_stringify(const lept_value* v, size_t* length) {
    lept_context c;
    assert(v != NULL);
    c.stack = (char*)malloc(c.size = LEPT_PARSE_STACK_INIT_SIZE);
    c.top = 0;
    lept_stringify_value(&c, v);
    if (length) 
        *length = c.top;
    PUTC(&c, '\0');
    return c.stack;
}

/***********************  方便测试函数 **************************************************/
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

lept_value* lept_get_object_value(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.obj.size);
    return &v->u.obj.m[index].val;
}
size_t lept_get_object_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    return v->u.obj.size;
}
const char* lept_get_object_key(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.obj.size);
    return v->u.obj.m[index].key;
}
size_t lept_get_object_key_length(const lept_value* v, size_t index) {
        assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.obj.size);
    return v->u.obj.m[index].klen;
}

#define LEPT_KEY_NOT_EXIST ((size_t)-1)

/* 线性搜寻 */
size_t lept_find_object_index(const lept_value* v, const char* key, size_t klen) {
    size_t i;
    assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
    for (i = 0; i < v->u.obj.size; i++)
        if (v->u.obj.m[i].klen == klen && memcpy(v->u.obj.m[i].key, key, klen))
            return i;
    return LEPT_KEY_NOT_EXIST;
}

lept_value* lept_find_object_value(lept_value* v, const char* key, size_t klen) {
    size_t index = lept_find_object_index(v, key, klen);
    return index != LEPT_KEY_NOT_EXIST ? &v->u.obj.m[index].val: NULL;
}


int lept_is_equal(const lept_value* lhs, const lept_value* rhs) {
}


/* 深拷贝 */
void lept_copy(lept_value* dst, const lept_value* src) {
    size_t i;
    assert(src != NULL && dst != NULL && src != dst);
    switch (src->type) {
    case LEPT_STRING:   lept_set_string(dst, src->u.str.s, src->u.str.len); break;
    case LEPT_ARRAY:    
        lept_set_array(dst, src->u.arr.size);
        for (i = 0; i < src->u.arr.size; i++)
            lept_copy(&dst->u.arr.e[i], &src->u.arr.e[i]);
        dst->u.arr.size = src->u.arr.size;
        break;
    case LEPT_OBJECT:
        lept_set_object(dst, src->u.obj.size);
        for (i = 0; i < src->u.obj.size; i++) {
            lept_value* val = lept_set_object_value(dst, src->u.obj.m[i].key, src->u.obj.m[i].klen);
            lept_copy(val, &src->u.obj.m[i].val);
        }
        dst->u.obj.size = src->u.obj.size;
        break;
    default:
        lept_free(dst);
        memcpy(dst, src, sizeof(lept_value));
        break;
    }
}


void lept_move(lept_value* dst, lept_value* src) {
    assert(dst != NULL && src != NULL && src != dst);
    lept_free(dst);
    memcpy(dst, src, sizeof(lept_value));
    lept_init(src);
}

void lept_swap(lept_value* lhs, lept_value* rhs) {
    assert(lhs != NULL && rhs != NULL);
    if (lhs != rhs) {
        lept_value temp;
        memcpy(&temp, lhs, sizeof(lept_value));
        memcpy(lhs, rhs, sizeof(lept_value));
        memcpy(rhs, &temp, sizeof(lept_value));
    }
}

void lept_set_array(lept_value* v, size_t capacity) {
    assert(v != NULL);
    lept_free(v);
    v->type = LEPT_ARRAY;
    v->u.arr.capacity = capacity;
    v->u.arr.size = 0;
    v->u.arr.e = capacity > 0 ? (lept_value*)malloc(capacity * sizeof(lept_value)) : NULL;
}
size_t lept_get_array_size(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.arr.size;
}
size_t lept_get_array_capacity(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.arr.capacity;
}

void lept_reserve_array(lept_value* v, size_t capacity) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->u.arr.capacity < capacity) {
        v->u.arr.capacity = capacity;
        v->u.arr.e = (lept_value*)realloc(v->u.arr.e, capacity * sizeof(lept_value));
    }
}
/* 容量缩小至刚好能放置现有元素 */
void lept_shrink_array(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->u.arr.capacity > v->u.arr.size) {
        v->u.arr.capacity = v->u.arr.size;
        v->u.arr.e = (lept_value*)realloc(v->u.arr.e, v->u.arr.size * sizeof(lept_value));
    }
}
void lept_clear_array(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    lept_erase_array_element(v, 0, v->u.arr.size);
}
lept_value* lept_get_array_element(const lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY && index < v->u.arr.size);
    return &v->u.arr.e[index];
}
lept_value* lept_pushback_array_element(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    if (v->u.arr.size == v->u.arr.capacity)
        lept_reserve_array(v, v->u.arr.capacity == 0 ? 1 : v->u.arr.capacity * 2);
    lept_init(&v->u.arr.e[v->u.arr.size]);
    return &v->u.arr.e[v->u.arr.size];
}
void lept_popback_array_element(lept_value* v) {
    assert(v != NULL && v->type == LEPT_ARRAY && v->u.arr.size >0);
    lept_free(&v->u.arr.e[--v->u.arr.size]);
}
lept_value* lept_insert_array_element(lept_value* v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY && index < v->u.arr.size);
    if (v->u.arr.size == v->u.arr.capacity)
        lept_reserve_array(v, v->u.arr.capacity == 0 ? 1 : v->u.arr.capacity * 2);
    memcpy(&v->u.arr.e[index+1], &v.u.arr.e[index], (v->u.arr.size - index) * sizeof(lept_value));
    v->u.arr.size++;
    return &v->u.arr.e[index];
}
void lept_erase_array_element(lept_value* v, size_t index, size_t count) {
    size_t i;
    assert(v != NULL && v->type == LEPT_ARRAY && index >= 0 && index + count <= v->u.arr.size);
    for (i = index; i < count; i++) 
        lept_free(&v->u.arr.e[i]);
    memcpy(v->u.arr.e + index, v->u.arr.e+index+count, count * sizeof(lept_value));
    for (i = v->u.arr.size - count; i < v->u.arr.size; i++)
        lept_init(&v->u.arr.e[i]);
    v->u.arr.size -= count;
}

void lept_set_object(lept_value* v, size_t capacity) {
    assert(v != NULL);
    lept_free(v);
    v->type = LEPT_OBJECT;
    v->u.obj.size = 0;
    v->u.obj.capacity = capacity;
    v->u.obj.m = capacity > 0 ? (lept_member*)malloc(capacity * sizeof(lept_member)) : NULL;
}

size_t lept_get_object_size(const lept_value* v);
size_t lept_get_object_capacity(const lept_value* v);
void lept_reserve_object(lept_value* v, size_t capacity);
void lept_shrink_object(lept_value* v);
void lept_clear_object(lept_value* v);
const char* lept_get_object_key(const lept_value* v, size_t index);
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(const lept_value* v, size_t index);
size_t lept_find_object_index(const lept_value* v, const char* key, size_t klen);
lept_value* lept_find_object_value(lept_value* v, const char* key, size_t klen);

/* 返回新增键值对的指针 */
lept_value* lept_set_object_value(lept_value* v, const char* key, size_t klen) {
    size_t index, i;
    assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
    index = lept_find_object_index(v, key, klen);
    if (index != LEPT_KEY_NOT_EXIST)
        return lept_get_object_value(v, index);
    /* 至此 说明不存在 . 首先判断 capacity 再插入*/
    if (v->u.obj.size == v->u.obj.capacity) 
        lept_reserve_object(v, v->u.obj.capacity == 0 ? 1 : v->u.obj.capacity * 2);
    
    i = v->u.obj.size;
    v->u.obj.m[i].key = (char*)malloc(klen+1);
    memcpy(v->u.obj.m[i].key, key, klen);
    v->u.obj.m[i].key[klen] = '\0';
    v->u.obj.m[i].klen = klen;
    lept_init(&v->u.obj.m[i].val);
    v->u.obj.size++;
    return &v.u.obj.m[i].val;
}


void lept_remove_object_value(lept_value* v, size_t index);