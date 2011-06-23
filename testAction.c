#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "action.h"

void test_action_simple(void);
void test_action_loop(void);

int main(int argc, char **argv) {
    printf("Running test_action_simple()\n");
    test_action_simple();

    printf("Running test_action_loop()\n");
    test_action_loop();
    return 0;
}

void test_action_simple(void) {
    char *buffer;
    Action action;

    buffer = malloc(20);
    memcpy(buffer, "thisrandomteststring", 20);

    action = bs_new_action(HASH_CHUNK, buffer);

    assert(memcmp(action->data, buffer, 20) == 0);
    assert(action->type == HASH_CHUNK);

    free(buffer);
    free(action);
}

void test_action_loop(void) {
    int i;
    char *buffer, *format = "thisisrandomteststringnumber%04d";
    Action action;

    buffer = malloc(32);

    for (i = 0; i < 10000; i++) {
        snprintf(buffer, 32, format, i);

        action = bs_new_action(HASH_CHUNK, buffer);

        assert(memcmp(action->data, buffer, 32) == 0);
        assert(action->type == HASH_CHUNK);
        free(action);
    }

    free(buffer);
}
