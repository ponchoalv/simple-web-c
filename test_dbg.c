#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "dbg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Redirect stderr to a file for testing
FILE* redirect_stderr_to_file(const char* filename) {
    fflush(stderr);
    return freopen(filename, "w", stderr);
}

// Read the content of a file into a buffer
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

// Test the debug macro
static void test_debug(void** state) {
    (void)state; // unused parameter

    FILE* file = redirect_stderr_to_file("stderr.txt");
    debug("Test debug message: %d", 1);
    fclose(file);

    char* output = read_file("stderr.txt");
    assert_non_null(strstr(output, "[DEBUG]"));
    assert_non_null(strstr(output, "Test debug message: 1"));
    free(output);
}

// Test the log_err macro
static void test_log_err(void** state) {
    (void)state; // unused parameter

    FILE* file = redirect_stderr_to_file("stderr.txt");
    log_err("Test error message: %d", 2);
    fclose(file);

    char* output = read_file("stderr.txt");
    assert_non_null(strstr(output, "[ERROR]"));
    assert_non_null(strstr(output, "Test error message: 2"));
    free(output);
}

// Test the log_warn macro
static void test_log_warn(void** state) {
    (void)state; // unused parameter

    FILE* file = redirect_stderr_to_file("stderr.txt");
    log_warn("Test warning message: %d", 3);
    fclose(file);

    char* output = read_file("stderr.txt");
    assert_non_null(strstr(output, "[WARN]"));
    assert_non_null(strstr(output, "Test warning message: 3"));
    free(output);
}

// Test the log_info macro
static void test_log_info(void** state) {
    (void)state; // unused parameter

    FILE* file = redirect_stderr_to_file("stderr.txt");
    log_info("Test info message: %d", 4);
    fclose(file);

    char* output = read_file("stderr.txt");
    assert_non_null(strstr(output, "[INFO]"));
    assert_non_null(strstr(output, "Test info message: 4"));
    free(output);
}

// Test the check macro
static void test_check(void** state) {
    (void)state; // unused parameter

    FILE* file = redirect_stderr_to_file("stderr.txt");
    int a = 0;
    if (!a) {
        log_err("Check failed: %d", a);
    }
    fclose(file);

    char* output = read_file("stderr.txt");
    assert_non_null(strstr(output, "[ERROR]"));
    assert_non_null(strstr(output, "Check failed: 0"));
    free(output);
}

// Test the check_mem macro
static void test_check_mem(void** state) {
    (void)state; // unused parameter

    FILE* file = redirect_stderr_to_file("stderr.txt");
    char* ptr = NULL;
    if (!ptr) {
        log_err("Out of memory.");
    }
    fclose(file);

    char* output = read_file("stderr.txt");
    assert_non_null(strstr(output, "[ERROR]"));
    assert_non_null(strstr(output, "Out of memory."));
    free(output);
}

// Test the check_debug macro
static void test_check_debug(void** state) {
    (void)state; // unused parameter

    FILE* file = redirect_stderr_to_file("stderr.txt");
    int a = 0;
    if (!a) {
        debug("Check debug failed: %d", a);
    }
    fclose(file);

    char* output = read_file("stderr.txt");
    assert_non_null(strstr(output, "[DEBUG]"));
    assert_non_null(strstr(output, "Check debug failed: 0"));
    free(output);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_debug),
        cmocka_unit_test(test_log_err),
        cmocka_unit_test(test_log_warn),
        cmocka_unit_test(test_log_info),
        cmocka_unit_test(test_check),
        cmocka_unit_test(test_check_mem),
        cmocka_unit_test(test_check_debug),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}