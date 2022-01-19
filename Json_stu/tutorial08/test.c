/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-09 20:29:59
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-19 21:13:56
 * @@Email: 8211180515@csu.edu.cn
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"
#include <assert.h>

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
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")


#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")


        /* fprintf(stdout, "-------%s:%d: actual %d: -------\n", __FILE__, __LINE__, lept_get_type(&val));\*/
#define TEST_PARSE(parse_ret, get_type, json) \
    do {\
        lept_value val;\
        lept_init(&val);\
        val.type = LEPT_FALSE;\
        EXPECT_EQ_INT(parse_ret, lept_parse(&val, json));\
        EXPECT_EQ_INT(get_type, lept_get_type(&val));\
        lept_free(&val);\
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

    /* invalid value in array */
#if 1
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL, "[1,]");
    TEST_PARSE(LEPT_PARSE_INVALID_VALUE, LEPT_NULL,"[\"a\", nul]");
#endif
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

#define TEST_STRING(expect, json) \
    do {\
        lept_value v;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
        EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v));\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
#if 1
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */

#endif
}

static void test_parse_missing_quotation_mark() {
    TEST_PARSE(LEPT_PARSE_MISS_QUOTATION_MARK, LEPT_NULL, "\"");
    TEST_PARSE(LEPT_PARSE_MISS_QUOTATION_MARK, LEPT_NULL, "\"abc");
}

static void test_parse_invalid_string_escape() {
#if 1
    TEST_PARSE(LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_NULL, "\"\\v\"");
    TEST_PARSE(LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_NULL, "\"\\'\"");
    TEST_PARSE(LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_NULL, "\"\\0\"");
    TEST_PARSE(LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_NULL, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() {
#if 1
    TEST_PARSE(LEPT_PARSE_INVALID_STRING_CHAR, LEPT_NULL, "\"\x01\"");
    TEST_PARSE(LEPT_PARSE_INVALID_STRING_CHAR, LEPT_NULL, "\"\x1F\"");
#endif
}

static void test_parse_invalid_unicode_hex() {
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL, "\"\\u\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u0\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u01\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u012\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u/000\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\uG000\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u0/00\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u0G00\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u00/0\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u00G0\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u000/\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_NULL,  "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() {
#if 1
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_SURROGATE, LEPT_NULL, "\"\\uD800\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_SURROGATE, LEPT_NULL, "\"\\uDBFF\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_SURROGATE, LEPT_NULL, "\"\\uD800\\\\\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_SURROGATE, LEPT_NULL, "\"\\uD800\\uDBFF\"");
    TEST_PARSE(LEPT_PARSE_INVALID_UNICODE_SURROGATE, LEPT_NULL, "\"\\uD800\\uE000\"");
#endif
}

/*
ANSI C（C89）并没有 size_t 打印方法，在 C99 则加入了 "%zu"，
但 VS2015 中才有，之前的 VC 版本使用非标准的 "%Iu"。
因此，下面的代码使用条件编译去区分 VC 和其他编译器。
虽然这部分不跨平台也不是 ANSI C 标准，但它只在测试程序中，不太影响程序库的跨平台性。
*/
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

static void test_parse_array() {
    lept_value v;
    size_t i, j;

    lept_init(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ ]"));
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ_SIZE_T(0, lept_get_array_size(&v));
    lept_free(&v);

    lept_init(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ_SIZE_T(5, lept_get_array_size(&v));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_array_element(&v, 0)));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_array_element(&v, 1)));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_array_element(&v, 2)));
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 3)));
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&v, 4)));

    EXPECT_EQ_DOUBLE(123.0, lept_get_number_val(lept_get_array_element(&v, 3)));
    EXPECT_EQ_STRING("abc", lept_get_string(lept_get_array_element(&v, 4)), lept_get_string_length(lept_get_array_element(&v, 4)));
    lept_free(&v);


    lept_init(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
    EXPECT_EQ_SIZE_T(4, lept_get_array_size(&v));
    for (i = 0; i < 4; i++) {
        lept_value* arr = lept_get_array_element(&v, i);
        EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(arr));
        EXPECT_EQ_SIZE_T(i, lept_get_array_size(arr));

        for ( j = 0; j < i; j++) {
            lept_value* e = lept_get_array_element(arr, j);
            EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
            EXPECT_EQ_DOUBLE((double)j, lept_get_number_val(e));
        }
    }
    lept_free(&v);
}

static void test_parse_miss_comma_or_square_bracket() {
#if 1
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, LEPT_NULL, "[1");
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, LEPT_NULL, "[1}");
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, LEPT_NULL, "[1 2");
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, LEPT_NULL,  "[[]");
#endif
}

void test_parse_object() {
#if 1
    lept_value v;
    size_t i;
    lept_init(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "{ }"));
    EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
    EXPECT_EQ_SIZE_T(0, lept_get_object_size(&v));
    lept_free(&v);

    lept_init(&v);
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v,
        " { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [ 1, 2, 3 ],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } "
    ));

    EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
    EXPECT_EQ_SIZE_T(7, lept_get_object_size(&v));
    EXPECT_EQ_STRING("n", lept_get_object_key(&v, 0), lept_get_object_key_length(&v, 0));
    EXPECT_EQ_INT(LEPT_NULL,   lept_get_type(lept_get_object_value(&v, 0)));
    EXPECT_EQ_STRING("f", lept_get_object_key(&v, 1), lept_get_object_key_length(&v, 1));
    EXPECT_EQ_INT(LEPT_FALSE,  lept_get_type(lept_get_object_value(&v, 1)));
    EXPECT_EQ_STRING("t", lept_get_object_key(&v, 2), lept_get_object_key_length(&v, 2));
    EXPECT_EQ_INT(LEPT_TRUE,   lept_get_type(lept_get_object_value(&v, 2)));
    EXPECT_EQ_STRING("i", lept_get_object_key(&v, 3), lept_get_object_key_length(&v, 3));
    EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_object_value(&v, 3)));
    EXPECT_EQ_DOUBLE(123.0, lept_get_number_val(lept_get_object_value(&v, 3)));
    EXPECT_EQ_STRING("s", lept_get_object_key(&v, 4), lept_get_object_key_length(&v, 4));
    EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_object_value(&v, 4)));
    EXPECT_EQ_STRING("abc", lept_get_string(lept_get_object_value(&v, 4)), lept_get_string_length(lept_get_object_value(&v, 4)));
    EXPECT_EQ_STRING("a", lept_get_object_key(&v, 5), lept_get_object_key_length(&v, 5));
    EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(lept_get_object_value(&v, 5)));
    EXPECT_EQ_SIZE_T(3, lept_get_array_size(lept_get_object_value(&v, 5)));
    for (i = 0; i < 3; i++) {
        lept_value* e = lept_get_array_element(lept_get_object_value(&v, 5), i);
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
        EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number_val(e));
    }
    EXPECT_EQ_STRING("o", lept_get_object_key(&v, 6), lept_get_object_key_length(&v, 6));
    {
        lept_value* o = lept_get_object_value(&v, 6);
        EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(o));
        for (i = 0; i < 3; i++) {
            lept_value* ov = lept_get_object_value(o, i);
            EXPECT_TRUE('1' + i == lept_get_object_key(o, i)[0]);
            EXPECT_EQ_SIZE_T(1, lept_get_object_key_length(o, i));
            EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(ov));
            EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number_val(ov));
        }
    }
    lept_free(&v);
#endif
}

static void test_parse_miss_key() {
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{:1,");
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{1:1,");
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{true:1,");
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{false:1,");
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{null:1,");
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{[]:1,");
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{{}:1,");
    TEST_PARSE(LEPT_PARSE_MISS_KEY, LEPT_NULL, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_PARSE(LEPT_PARSE_MISS_COLON, LEPT_NULL, "{\"a\"}");
    TEST_PARSE(LEPT_PARSE_MISS_COLON, LEPT_NULL, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, LEPT_NULL, "{\"a\":1");
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, LEPT_NULL, "{\"a\":1]");
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, LEPT_NULL, "{\"a\":1 \"b\"");
    TEST_PARSE(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, LEPT_NULL, "{\"a\":{}");
}

/*********************** JSON generator  **********************************************************/
/* 把一个 JSON 解析，然后再生成另一 JSON，逐字符比较两个 JSON 是否一模一样。
这种测试可称为往返（roundtrip）测试 */
#define TEST_ROUNDTRIP(json)\
    do {\
        lept_value v;\
        char* json2;\
        size_t length;\
        lept_init(&v);\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        json2 = lept_stringify(&v, &length);\
        EXPECT_EQ_STRING(json, json2, length);\
        lept_free(&v);\
        free(json2);\
    } while(0)

static void test_stringify_number() {
    TEST_ROUNDTRIP("0");
    TEST_ROUNDTRIP("-0");
    TEST_ROUNDTRIP("1");
    TEST_ROUNDTRIP("-1");
    TEST_ROUNDTRIP("1.5");
    TEST_ROUNDTRIP("-1.5");
    TEST_ROUNDTRIP("3.25");
    TEST_ROUNDTRIP("1e+20");
    TEST_ROUNDTRIP("1.234e+20");
    TEST_ROUNDTRIP("1.234e-20");

    TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
    TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
    TEST_ROUNDTRIP("-4.9406564584124654e-324");
    TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
    TEST_ROUNDTRIP("-2.2250738585072009e-308");
    TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
    TEST_ROUNDTRIP("-2.2250738585072014e-308");
    TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
    TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
    TEST_ROUNDTRIP("\"\"");
    TEST_ROUNDTRIP("\"Hello\"");
    TEST_ROUNDTRIP("\"Hello\\nWorld\"");
    TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_ROUNDTRIP("\"Hello\\u0000World\"");

#if 0
    /* 值得注意的是此处：
    之前是在ASCII里储存UNICODE的码点，所以才需要转义，
    而现在输出UTF8编码的字符流，就不用转义了　*/
    TEST_ROUNDTRIP("\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_ROUNDTRIP("\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_ROUNDTRIP("\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_ROUNDTRIP("\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_ROUNDTRIP("\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
#endif
}
static void test_stringify_array() {
    TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}
static void test_stringify_object() {
    TEST_ROUNDTRIP("{}");
    TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    /* ... */
    test_stringify_number();
    test_stringify_string();
    test_stringify_array();
    test_stringify_object();
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

    test_parse_string();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();

    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();

    test_parse_array();
    test_parse_miss_comma_or_square_bracket();

#if 1
    test_parse_object();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
#endif
}


/*********************** access 测试内存泄露  **********************************************************/
/* ，我们故意先把值设为字符串，这么做可以测试设置其他类型时，
有没有调用 lept_free() 去释放内存。*/
static void test_access_boolean() {
    lept_value v;
    /* lept_init(&v); // 当忘记 init 导致 set 时 free 失败， 可通过 valgrind ./leptjson_test  检测出 */
    lept_set_string(&v, "a", 1);
    lept_set_boolean(&v, 1);
    EXPECT_TRUE(lept_get_boolean(&v));
    lept_set_boolean(&v, 0);
    EXPECT_FALSE(lept_get_boolean(&v));
    lept_free(&v);
}

static void test_access() {
    test_access_boolean();
}

int main(int argc, char const *argv[])
{
    test_parse();
    test_stringify();
    test_equal();
    test_copy();
    test_move();
    test_swap();
    test_access();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_cnt, test_pass * 100.0 / test_cnt);
    return main_ret;
}
