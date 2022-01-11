/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 20:29:59
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-11 22:56:32
 * @@Email: 8211180515@csu.edu.cn
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_cnt = 0;
static int test_pass = 0;

/* 此处的 do while(0) 是为了囊括所有的语句 */
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_cnt++;\
        if (equality) {\
            test_pass++;\
        } else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")


#define TEST_PARSE(parse_ret, get_type, json) \
    do {\
        lept_value val;\
        val.type = LEPT_FALSE;\
        EXPECT_EQ_INT(parse_ret, lept_parse(&val, json));\
        EXPECT_EQ_INT(get_type, lept_get_type(&val));\
    } while(0)


static void test_parse_null() {
    TEST_PARSE(LEPT_PARSE_OK, LEPT_NULL, "null");
}

static void test_parse_true() {
    TEST_PARSE(LEPT_PARSE_OK, LEPT_TRUE, "true");
}


static void test_parse_false() {
    TEST_PARSE(LEPT_PARSE_OK, LEPT_FALSE, "false");
}

static void test_parse_expect_value() {
    TEST_PARSE(LEPT_PARSE_EXPECT_VALUE, LEPT_NULL, "");
    TEST_PARSE(LEPT_PARSE_EXPECT_VALUE, LEPT_NULL, " ");
}

static void test_parse_invalid_value() {
    /* invalid character */
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "nul");
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "?");

    /* invalid number */
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "+0");
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "+1");
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, ".123"); /* at least one digit before '.' */
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "1.");   /* at least one digit after '.' */
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "INF");
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "inf");
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "NAN");
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "nan");
}

static void test_parse_root_not_singular() {
    TEST_PARSE(LEPT_PARSE_ROOT_NOT_SINGULAR, LEPT_NULL, "null x");

    /* invalid number */
    TEST_PARSE(LEPT_PARSE_ROOT_NOT_SINGULAR, LEPT_NULL, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_PARSE(LEPT_PARSE_ROOT_NOT_SINGULAR, LEPT_NULL, "0x0");
    TEST_PARSE(LEPT_PARSE_ROOT_NOT_SINGULAR, LEPT_NULL, "0x123");
}

#define TEST_NUMBER(expect, json) \
    do {\
        lept_value v;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, lept_get_number_val(&v));\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");

}

static void test_parse_number_too_big() {
    TEST_PARSE(LEPT_PARSE_NUMBER_TOO_BIG, LEPT_NULL, "1e309");
    TEST_PARSE(LEPT_PARSE_NUMBER_TOO_BIG, LEPT_NULL, "-1e309");
}

/*  static 函数的意思是指，该函数只作用于编译单元中，那么没有被调用时，编译器是能发现的。 */
static void test_parse() {
    test_parse_null();
    test_parse_false();
    test_parse_true();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();

    test_parse_number();
    test_parse_number_too_big();
}

int main(int argc, char const *argv[])
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_cnt, test_pass * 100.0 / test_cnt);
    return main_ret;
}
