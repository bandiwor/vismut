#include "vismut_test.h"
#include <stdio.h>
#include <time.h>

VismutTestNode *g_vismut_test_list = NULL;
int g_vismut_tests_run = 0;
int g_vismut_tests_failed = 0;
const char *g_vismut_current_test = NULL;

int VismutTest_RunAll(void) {
    VismutTestNode *curr = g_vismut_test_list;
    int passed = 0;

    printf("========== RUNNING VISMUT TESTS ==========\n");

    while (curr) {
        g_vismut_current_test = curr->test_name;
        const int failed_before = g_vismut_tests_failed;

        printf("[ RUN      ] %s.%s\n", curr->suite_name, curr->test_name);
        curr->func();

        if (g_vismut_tests_failed == failed_before) {
            printf("[       OK ] %s.%s\n", curr->suite_name, curr->test_name);
            passed++;
        }

        g_vismut_tests_run++;
        curr = curr->next;
    }

    printf("==========================================\n");
    printf("Tests run:    %d\n", g_vismut_tests_run);
    printf("Tests passed: %d\n", passed);
    printf("Tests failed: %d\n", g_vismut_tests_failed);

    return g_vismut_tests_failed > 0 ? 1 : 0;
}

int main(void) {
    VismutTest_RunAll();
}
