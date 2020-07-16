
#define ZEN_DS_IMPLEMENTATION
#include "zen_ds.h"

#include <string.h>

// the argument against this is that it's also so complicated.  A lot going on and we
// would have to read and understand all the damn code.  So really, what we need is the
// exact same kind of thing, but simple.  Which is what the bitwise thing is.
//
// Let's just do that sucker as an stb header and be done with it.
void buf_test(void) {
    int *buf = NULL;
    assert(buf_len(buf) == 0);
    int n = 1024;
    for (int i = 0; i < n; i++) {
        buf_push(buf, i);
    }
    assert(buf_len(buf) == n);
    for (size_t i = 0; i < buf_len(buf); i++) {
        assert(buf[i] == i);
    }
    buf_free(buf);
    assert(buf == NULL);
    assert(buf_len(buf) == 0);
    char *str = NULL;
    buf_printf(str, "One: %d\n", 1);
    assert(strcmp(str, "One: 1\n") == 0);
    buf_printf(str, "Hex: 0x%x\n", 0x12345678);
    assert(strcmp(str, "One: 1\nHex: 0x12345678\n") == 0);
}

void map_test(void) {
    Map map = {0};
    enum { N = 1024 };
    for (size_t i = 1; i < N; i++) {
        map_put(&map, (void *)i, (void *)(i+1));
    }
    for (size_t i = 1; i < N; i++) {
        void *val = map_get(&map, (void *)i);
        assert(val == (void *)(i+1));
    }
}

int main(int argc, char **argv) {
    printf("starting test...\n");
    buf_test();
    map_test();
    printf("test conplete\n");
    return 0;
}
