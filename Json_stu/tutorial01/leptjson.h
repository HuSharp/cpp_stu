/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 14:05:39
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-10 13:08:30
 * @@Email: 8211180515@csu.edu.cn
 */
#ifndef LEPJSON_H__
#define LEPJSON_H__

/* JSON 只包含 6 种数据类型 */
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/* JSON 是一个树形结构，我们最终需要实现一个树的数据结构，
每个节点使用 lept_value 结构体表示，我们会称它为一个 JSON 值（JSON value）。*/
typedef struct {
    lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,        /* 一个 JSON 只含有空白 */
    LEPT_PARSE_INVALID_VALUE,       /* 在空白之后还有其他字符 */
    LEPT_PARSE_ROOT_NOT_SINGULAR    /* 若值不是那三种字面值, 现时的值只可以是 null、false 或 true，它们分别有对应的字面值（literal）*/
};

/* parse JSON */
int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);
void lept_set_type(lept_value* v, lept_type t);

#endif