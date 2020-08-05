#include "greatest/greatest.h"
#include <inttypes.h>
#include <assert.h>

#define ZENDS_STATIC_ARRAY_IMPLEMENTATION
#include "zends_static_array.h"

// This seems like a lot of work to do this...

typedef struct object_t {
    int x, y;
    int id;
} object_t;

enum {MAX_ITEMS = 32};
uint32_t num_items;
object_t items[MAX_ITEMS];


#define ZENDS_SBUF_TYPE(var, type, size) \
struct { \
    type list[size]; \
    uint32_t num; \
    const uint32_t max; \
} var = { \
    .max = size, \
} \

#define zends_sbuf_push(buf, val) (assert((buf)->num + 1 < (buf)->max), (buf)->list[(buf)->num++] = (val))
#define zends_sbuf_pop(buf) (assert((buf)->num > 0), (buf)->list[--((buf)->num)])
#define zends_sbuf_erase(buf, index) assert((index) < (buf)->num), (buf)->list[(index)] = (buf)->list[--((buf)->num)]
#define zends_sbuf_clear(buf) ((buf)->num = 0)

static ZENDS_SBUF_TYPE(array, uint32_t, 32);

typedef struct player_t {
    uint32_t id;
    const char *name; // just pretend it's interned.
} player_t;

ZENDS_SBUF_TYPE(player_list, player_t, 32);

static void setup(void *arg) {
    array.num = 0;
}

static void teardown(void *arg) {

}

// so here we go trying out a newer idea
// sbuf2 just like sbuf but even better?
// still need some kind of header

// another idea could be we store all the info we need
// in the array as bytes at the beginning.  
// Well that's crazy and interesting.  Same really as the
// stretchy buffer, we still have that overhead of everything.

// we need length and capacity, so two size_t values in addition
// to the actual number of thingsy.



TEST wish_i_could(void) {
    // So what I wish I could write is:
    // so we can't really init the thing in a function.
    // we have to create the shite somewhere else
#if 0
    sbuf_push(list, 42);
    ASSERT_EQ(list[0], 42);
    for (int i = 0; i < sbuf_len(list); ++i) {
        ASSERT_EQ(i+1, list[i]);
    }
#endif
    PASS();
}

// TEST test_asserts(void) {
//     // array.num = MAX_MY_ARRAY_SIZE;
//     // push_back(&array, 42);
//     // pop_back(&array);
//     swap_erase(&array, 0);
//     PASS();
// }

TEST test_player_list(void) {

    zends_sbuf_push(&player_list, ((player_t) {
        .id = 42, .name = "ASDF" 
    }));

    player_t p = {
        .id = 8,
        .name = "P8",
    };
    zends_sbuf_push(&player_list, p);

    p.id = 9;
    p.name = "P9";
    zends_sbuf_push(&player_list, p);

    ASSERT_EQ(3, player_list.num);

    ASSERT_EQ(42, player_list.list[0].id);
    ASSERT_STR_EQ("ASDF", player_list.list[0].name);

    ASSERT_EQ(8, player_list.list[1].id);
    ASSERT_STR_EQ("P8", player_list.list[1].name);

    ASSERT_EQ(9, player_list.list[2].id);
    ASSERT_STR_EQ("P9", player_list.list[2].name);

    PASS();
}

TEST smoke_screen(void) {

    for (int i = 0; i < 10; ++i) {
        zends_sbuf_push(&array, i+1);
    } 
    ASSERT_EQ(10, array.num);

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(i+1, array.list[i]);
    }

    for (int i = 10 - 1; i >=0 ; --i) {
        ASSERT_EQ(i+1, zends_sbuf_pop(&array));
    }
    ASSERT_EQ(0, array.num);

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(i+1, array.list[i]);
    }
    zends_sbuf_clear(&array);
    ASSERT_EQ(0, array.num);

    zends_sbuf_push(&array, 11); // idx 0
    zends_sbuf_push(&array, 22); // idx 1
    zends_sbuf_push(&array, 33); // idx 2
    zends_sbuf_push(&array, 44); // idx 3
    zends_sbuf_push(&array, 55); // idx 4

    // {11,22,55,44}
    zends_sbuf_erase(&array, 2);
    ASSERT_EQ(4, array.num);

    ASSERT_EQ(11, array.list[0]);
    ASSERT_EQ(22, array.list[1]);
    ASSERT_EQ(55, array.list[2]);
    ASSERT_EQ(44, array.list[3]);

    // {11,22,55}
    zends_sbuf_erase(&array, 3);
    ASSERT_EQ(3, array.num);
    ASSERT_EQ(11, array.list[0]);
    ASSERT_EQ(22, array.list[1]);
    ASSERT_EQ(55, array.list[2]);

    // {55,22}
    zends_sbuf_erase(&array, 0);
    ASSERT_EQ(2, array.num);
    ASSERT_EQ(55, array.list[0]);
    ASSERT_EQ(22, array.list[1]);

    // {22}
    zends_sbuf_erase(&array, 0);
    ASSERT_EQ(1, array.num);
    ASSERT_EQ(22, array.list[0]);

    // {}
    zends_sbuf_erase(&array, 0);
    ASSERT_EQ(0, array.num);

    PASS();
}

GREATEST_SUITE(test_static_array) {
    GREATEST_SET_SETUP_CB(setup, NULL);
    GREATEST_SET_TEARDOWN_CB(teardown, NULL);

    // RUN_TEST(test_asserts);
    RUN_TEST(test_player_list);
    RUN_TEST(wish_i_could);
    RUN_TEST(smoke_screen);
}

