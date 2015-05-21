#ifndef _TEST_H_
#define _TEST_H_

#define TEST_ROUNDS 1
// #define PR_DEBUG

char *new_str(const char str[], unsigned len);
void bitfield_test();

#define pr_test(fmt, ...) printf("%s " fmt "\n", __func__, ##__VA_ARGS__)

#define test_pass_round(res, s) do { \
    res.failed += res.rfail; \
    res.passed += res.rpass; \
    printf("[ %s ] %s\n", res.rfail ? "errors" : "passed", s); \
    res.rfail = 0; \
    res.rpass = 0; \
} while(0)

#define test_do(fcond, act, fmt, ...) do { \
    if(fcond) { \
        (res)->rfail++; \
        printf("%s fail: " fmt "!\n", __func__, ##__VA_ARGS__); \
        act; \
    } else { \
        (res)->rpass++; \
    }} while(0)

#define test_try(fcond, fmt, ...) test_do(fcond, nop(), fmt, ##__VA_ARGS__)
#define test_fail(fcond, fmt, ...) test_do(fcond, return, fmt, ##__VA_ARGS__)
#define test_break(fcond, fmt, ...) test_do(fcond, break, fmt, ##__VA_ARGS__)

#define test_result_new(name) { 0, 0, 0, 0, 0, name }

struct test_result_s {
    unsigned passed, failed, rpass, rfail;
    float rate;
    const char *name;
};

#endif
