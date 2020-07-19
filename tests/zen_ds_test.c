#define ZEN_DS_IMPLEMENTATION
#include "zen/zen_ds.h"

#include <string.h>

void buf_test(void) {
    int *buf = NULL;
    assert(zends_buf_len(buf) == 0);
    int n = 1024;
    for (int i = 0; i < n; i++) {
        zends_buf_push(buf, i);
    }
    assert(zends_buf_len(buf) == n);
    for (size_t i = 0; i < zends_buf_len(buf); i++) {
        assert(buf[i] == i);
    }
    zends_buf_free(buf);
    assert(buf == NULL);
    assert(zends_buf_len(buf) == 0);
    char *str = NULL;
    zends_buf_printf(str, "One: %d\n", 1);
    assert(strcmp(str, "One: 1\n") == 0);
    zends_buf_printf(str, "Hex: 0x%x\n", 0x12345678);
    assert(strcmp(str, "One: 1\nHex: 0x12345678\n") == 0);
}

void map_test(void) {
    zends_map_t map = {0};
    enum { N = 1024 };
    for (size_t i = 1; i < N; i++) {
        zends_map_put(&map, (void *)i, (void *)(i+1));
    }
    for (size_t i = 1; i < N; i++) {
        void *val = zends_map_get(&map, (void *)i);
        assert(val == (void *)(i+1));
    }
}

int main(int argc, char **argv) {
    printf("starting test...\n");
    buf_test();
    map_test();
    zends_str_free();
    printf("test conplete\n");
    return 0;
}
