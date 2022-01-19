/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 14:05:39
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-19 23:32:15
 * @@Email: 8211180515@csu.edu.cn
 */
#ifndef LEPJSON_H__
#define LEPJSON_H__

#include <stddef.h> /* size_t */

/* JSON 只包含 6 种数据类型 */
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/* 
JSON 是一个树形结构，我们最终需要实现一个树的数据结构，
每个节点使用 lept_value 结构体表示，我们会称它为一个 JSON 值（JSON value）。
仅当 type == LEPT_NUMBER 时，n 才表示 JSON 数字的数值
一个值不可能同时为数字和字符串，因此我们可使用 C 语言的 union 来节省内存
object 选择用动态数组的方案
*/
typedef struct lept_value lept_value;   /* 由于 lept_value 内使用了自身类型的指针，我们必须前向声明（forward declare）此类型。 */
typedef struct lept_member lept_member;
struct lept_value {
    lept_type type;
    union {
        struct { lept_member* m; size_t size, capacity; }obj; /* object */
        struct { lept_value* e; size_t size, capacity; }arr;  /* capacity 字段，表示当前已分配的元素数目;  size 则表示现时的有效元素数目. */
        struct { char* s; size_t len;}str;          /* string */
        double n;                                   /* number */
    }u;
};

struct lept_member {
    char* key;
    size_t klen;    /* key's length */
    lept_value val;
};


enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,                /* 一个 JSON 只含有空白 */
    LEPT_PARSE_INVALID_VALUE,               /* 在空白之后还有其他字符 */
    LEPT_PARSE_ROOT_NOT_SINGULAR,           /* 若值不是那三种字面值, 现时的值只可以是 null、false 或 true，它们分别有对应的字面值（literal）*/
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_SURROGATE_OK,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,    /* 只有高代理项而欠缺低代理项，或是低代理项不在合法码点范围 */
    LEPT_PARSE_INVALID_UNICODE_HEX,          /* \u 后不是 4 位十六进位数字 */
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    LEPT_PARSE_MISS_KEY,                     /* object 没有 key */
    LEPT_PARSE_MISS_COLON,                   /* object 没有  */
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};


void lept_free(lept_value* v);
void lept_init(lept_value* v);

/* parse JSON */
int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);
int lept_is_equal(const lept_value* lhs, const lept_value* rhs);

void lept_set_type(lept_value* v, lept_type t);

const char* lept_get_string(const lept_value* v);
int lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);

#define lept_set_null(v) lept_free(v)

int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

double lept_get_number_val(const lept_value* v);
void lept_set_number(lept_value* v, double n);

/* JSON generator */
char* lept_stringify(const lept_value* v, size_t* length);

/* other function */

void lept_set_array(lept_value* v, size_t capacity);
size_t lept_get_array_size(const lept_value* v);
size_t lept_get_array_capacity(const lept_value* v);
void lept_reserve_array(lept_value* v, size_t capacity);
void lept_shrink_array(lept_value* v);
void lept_clear_array(lept_value* v);
lept_value* lept_get_array_element(const lept_value* v, size_t index);
lept_value* lept_pushback_array_element(lept_value* v);
void lept_popback_array_element(lept_value* v);
lept_value* lept_insert_array_element(lept_value* v, size_t index);
void lept_erase_array_element(lept_value* v, size_t index, size_t count);

void lept_set_object(lept_value* v, size_t capacity);
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
lept_value* lept_set_object_value(lept_value* v, const char* key, size_t klen);
void lept_remove_object_value(lept_value* v, size_t index);


void lept_copy(lept_value* dst, const lept_value* src);
void lept_move(lept_value* dst, lept_value* src);
void lept_swap(lept_value* lhs, lept_value* rhs);

#endif