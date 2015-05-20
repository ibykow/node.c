#ifndef _TEST_H_
#define _TEST_H_

#define TEST_ROUNDS 10
// #define PR_DEBUG

char *new_str(const char str[], unsigned len);
void bitfield_test();

#define pr_test(fmt, ...) printf("%s " fmt "\n", __func__, ##__VA_ARGS__)
#define test_do(res, fcond, act, fmt, ...) do { \
    if(fcond) { \
        (res)->failed++; \
        printf("%s fail: " fmt "!\n", __func__, ##__VA_ARGS__); \
        act; \
    } else { \
        (res)->passed++; \
    }} while(0)

#define test_try(res, fcond, fmt, ...) test_do(res, fcond, nop(), fmt, ##__VA_ARGS__)
#define test_fail(res, fcond, fmt, ...) test_do(res, fcond, return, fmt, ##__VA_ARGS__)
#define test_break(res, fcond, fmt, ...) test_do(res, fcond, break, fmt, ##__VA_ARGS__)

#endif
