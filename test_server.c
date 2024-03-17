#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "server.h" // Asegúrate de que server.h exista y declare las funciones que estás probando

void test_get_file_case_insensitive(void **state) {
    (void) state;  // No se usa en esta prueba

    char *result = get_file_case_insensitive("testfile");
    assert_string_equal(result, "TestFile");
}

void test_url_decode(void **state) {
    (void) state;  // No se usa en esta prueba

    char *result = url_decode("Hello%20World%21");
    assert_string_equal(result, "Hello World!");
    free(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_get_file_case_insensitive),
        cmocka_unit_test(test_url_decode),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}