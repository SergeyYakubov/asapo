#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef ASAPO_TESTS_AUTOMATIC_COMMON_CPP_INCLUDE_TESTING_C_H_
#define ASAPO_TESTS_AUTOMATIC_COMMON_CPP_INCLUDE_TESTING_C_H_

#define EXIT_IF_ERROR(...) exit_if_error_(__VA_ARGS__,__LINE__)
#define ASSERT_EQ_INT(...) assert_eq_int_(__VA_ARGS__,__LINE__)
#define ASSERT_EQ_STRING(...) assert_eq_string_(__VA_ARGS__,__LINE__)
#define ASSERT_TRUE(...) assert_true_(__VA_ARGS__,__LINE__)

void assert_eq_int_(uint64_t expected, uint64_t got, const char* message, int line) {
    printf("asserting %s at %d\n", message, line);
    if (expected != got) {
        printf("%s: expected %llu got %llu at %d\n", message, (unsigned long long)expected, (unsigned long long)got, line);
        exit(EXIT_FAILURE);
    }
    printf("asserting %s at %d - OK\n", message, line);

}

void assert_eq_string_(const char* expected, const char* got, const char* message, int line) {
    printf("asserting %s at %d\n", message, line);
    if (strcmp(expected, got) != 0) {
        printf("%s: expected %s got %s at %d\n", message, expected, got, line);
        exit(EXIT_FAILURE);
    }
    printf("asserting %s at %d - OK \n", message, line);

}

void assert_true_(int value, const char* message, int line) {
    printf("asserting %s at %d\n", message, line);
    if (value != 1) {
        printf("%s failed at %d\n", message, line);
        exit(EXIT_FAILURE);
    }
    printf("asserting %s at %d - OK \n", message, line);
}

void exit_if_error_(const char* error_string, const AsapoErrorHandle err, int line) {
    printf("asserting no error for %s at %d\n", error_string, line);
    if (asapo_is_error(err)) {
        char buf[1024];
        asapo_error_explain(err, buf, sizeof(buf));
        printf("%s %s\n", error_string, buf);
        exit(EXIT_FAILURE);
    }
    printf("asserting no error for %s at %d - OK \n", error_string, line);
}



#endif //ASAPO_TESTS_AUTOMATIC_COMMON_CPP_INCLUDE_TESTING_C_H_
