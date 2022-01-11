/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 21:39:51
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-11 22:54:13
 * @@Email: 8211180515@csu.edu.cn
 */
#include "leptjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch)   do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char *json;
}lept_context;

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
    return v->n;
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
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }

    c->json = pos;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':   return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':   return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':   return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:    return lept_parse_number(c, v);
        case '\0':  return LEPT_PARSE_EXPECT_VALUE;
    }
}

/* parse JSON */
/* 提示：这里应该是 JSON-text = ws value ws */
/* 以下实现没处理最后的 ws 和 LEPT_PARSE_ROOT_NOT_SINGULAR */
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v !=  NULL);
    c.json = json;
    v->type = LEPT_NULL;

    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
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