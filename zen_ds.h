/* zen_ds.h - v0.0 - public domain data structures - Tim Wright 2020

    TODO:
    Need to redo all the zen libraries.
    - move over everything currently in there to an archive branch
    - then we can start again, and we start with just this fucking thing.
        zen_ds.h
      Because this is literaly the thing I always need for my project.
      I'm sure there are lots more.
    

   This is a single-header-file library that provides easy-to-use
   dynamic arrays and hash tables for C 

TBD: (also works in C++).

   To use this library, do this in *one* C or C++ file:
      #define ZEN_DS_IMPLEMENTATION
      #include "zen_ds.h"

 Code is taken from:
    https://github.com/pervognsen/bitwise/blob/master/ion/common.c

    It is just refactored into a single header file and some macros added for
    individual customizations.

 Why:
    I always need a dynamic array array, hash table, and string interning.
    I don't want a huge complicated thing.  I would rather understand all the
    code in case I need to make changes.

 Reinvent the Wheel?
    Yes, because I like doing stuff myself.  I find it fun.  Don't judge me.  XD

TABLE OF CONTENTS

  Table of Contents
  Compile-time options
  License
  Documentation
  Notes
  Notes - Dynamic arrays
  Notes - Hash maps
  Credits

COMPILE-TIME OPTIONS

    TODO: This is just an idea from stb_ds.h

  #define ZENDS_REALLOC(context,ptr,size) better_realloc
  #define ZENDS_FREE(context,ptr)         better_free

     These defines only need to be set in the file containing #define STB_DS_IMPLEMENTATION.

     By default stb_ds uses stdlib realloc() and free() for memory management. You can
     substitute your own functions instead by defining these symbols. You must either
     define both, or neither. Note that at the moment, 'context' will always be NULL.
     @TODO add an array/hash initialization function that takes a memory context pointer.

  #define ZENDS_UNIT_TESTS

     Defines a function stbds_unit_tests() that checks the functioning of the data structures.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

DOCUMENTATION
    TODO

NOTES
    TODO

NOTES - DYNAMIC ARRAY
    TODO 

NOTES - HASH MAP
    TODO

CREDITS

    Per Vognsen  -- API/implementation code
    Sean Barrett -- library, idea for dynamic array API/implementation

  Bugfixes:


*/

#ifndef INCLUDE_ZEN_DS_H
#define INCLUDE_ZEN_DS_H

#include <stdint.h>
#include <stdbool.h>
// #include <stddef.h>

// TODO: This check is kindof silly right?
#if defined(ZENDS_MALLOC)  && !(defined(ZENDS_CALLOC) && defined(ZENDS_REALLOC) && defined(ZENDS_FREE)) || \
    defined(ZENDS_CALLOC)  && !(defined(ZENDS_MALLOC) && defined(ZENDS_REALLOC) && defined(ZENDS_FREE)) || \
    defined(ZENDS_REALLOC) && !(defined(ZENDS_MALLOC) && defined(ZENDS_CALLOC)  && defined(ZENDS_FREE)) || \
    defined(ZENDS_FREE)    && !(defined(ZENDS_MALLOC) && defined(ZENDS_CALLOC)  && defined(ZENDS_REALLOC))
    #error "You must define all or none: ZENDS_MALLOC, ZENDS_CALLOC, ZENDS_REALLOC, ZENDS_FREE"
#endif
#if !(defined(ZENDS_MALLOC) || defined(ZENDS_CALLOC) || defined(ZENDS_REALLOC) || defined(ZENDS_FREE))
    #include <stdlib.h>
    #define ZENDS_MALLOC(s)     malloc(s)
    #define ZENDS_CALLOC(n,s)   calloc(n,s)
    #define ZENDS_REALLOC(p,s)  realloc(p,s)
    #define ZENDS_FREE(p)       free(p)
#endif


// stretchy buffer
// TODO: rename this suckers
// TODO: Also, do we want short names or something?
//
typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[];
} BufHdr;

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size);
char *buf__printf(char *buf, const char *fmt, ...);


// Maybe we don't even use the allocator here.
// // Arena allocator
// //
// #define ARENA_ALIGNMENT 8
// #define ARENA_BLOCK_SIZE (1024 * 1024)
// // #define ARENA_BLOCK_SIZE 1024
// // TODO: Rename the structs and functions
// // zends_arena_grow
// // zends_arena_t for the structs.  Something like that.
typedef struct Arena {
    char *ptr;
    char *end;
    char **blocks;
} Arena;

void *arena_alloc(Arena *arena, size_t size);

//Hash functions

uint64_t hash_uint64(uint64_t x);
uint64_t hash_ptr(const void *ptr);
uint64_t hash_mix(uint64_t x, uint64_t y);
uint64_t hash_bytes(const void *ptr, size_t len);

// Hash Map
// TODO: not sure if all the functions are actually part
// of the public api?

// TODO: Rename the structs and functions
typedef struct Map {
    uint64_t *keys;
    uint64_t *vals;
    size_t len;
    size_t cap;
} Map;

void *map_get(Map *map, const void *key);
void map_put(Map *map, const void *key, void *val);
uint64_t map_get_uint64(Map *map, void *key);
void map_put_uint64(Map *map, void *key, uint64_t val);
void *map_get_from_uint64(Map *map, uint64_t key);
void map_put_from_uint64(Map *map, uint64_t key, void *val);

// String interning
// TODO: Rename the structs and functions
const char *str_intern(const char *str);
const char *str_intern_range(const char *start, const char *end);

#endif

// ------------------------------------------
// Implementation
// ------------------------------------------
#ifdef ZEN_DS_IMPLEMENTATION

#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#define ZENDS_MIN(x, y) ((x) <= (y) ? (x) : (y))
#define ZENDS_MAX(x, y) ((x) >= (y) ? (x) : (y))
#define ZENDS_CLAMP_MAX(x, max) ZENDS_MIN(x, max)
#define ZENDS_CLAMP_MIN(x, min) ZENDS_MAX(x, min)
#define ZENDS_IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define ZENDS_ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ZENDS_ALIGN_UP(n, a) ZENDS_ALIGN_DOWN((n) + (a) - 1, (a))
#define ZENDS_ALIGN_DOWN_PTR(p, a) ((void *)ZENDS_ALIGN_DOWN((uintptr_t)(p), (a)))
#define ZENDS_ALIGN_UP_PTR(p, a) ((void *)ZENDS_ALIGN_UP((uintptr_t)(p), (a)))


void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert(buf_cap(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = ZENDS_CLAMP_MIN(2*buf_cap(buf), ZENDS_MAX(new_len, 16));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf))/elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap*elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = ZENDS_REALLOC(buf__hdr(buf), new_size);
    } else {
        new_hdr = ZENDS_MALLOC(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

char *buf__printf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t cap = buf_cap(buf) - buf_len(buf);
    size_t n = 1 + vsnprintf(buf_end(buf), cap, fmt, args);
    va_end(args);
    if (n > cap) {
        buf_fit(buf, n + buf_len(buf));
        va_start(args, fmt);
        size_t new_cap = buf_cap(buf) - buf_len(buf);
        n = 1 + vsnprintf(buf_end(buf), new_cap, fmt, args);
        assert(n <= new_cap);
        va_end(args);
    }
    buf__hdr(buf)->len += n - 1;
    return buf;
}

// Arena allocator
#define ARENA_ALIGNMENT 8
#define ARENA_BLOCK_SIZE (1024 * 1024)
// #define ARENA_BLOCK_SIZE 1024
// TODO: Rename the structs and functions
// zends_arena_grow
// zends_arena_t for the structs.  Something like that.

void arena_grow(Arena *arena, size_t min_size) {
    size_t size = ZENDS_ALIGN_UP(ZENDS_CLAMP_MIN(min_size, ARENA_BLOCK_SIZE), ARENA_ALIGNMENT);
    arena->ptr = ZENDS_MALLOC(size);
    assert(arena->ptr == ZENDS_ALIGN_DOWN_PTR(arena->ptr, ARENA_ALIGNMENT));
    arena->end = arena->ptr + size;
    buf_push(arena->blocks, arena->ptr);
}

void *arena_alloc(Arena *arena, size_t size) {
    if (size > (size_t)(arena->end - arena->ptr)) {
        arena_grow(arena, size);
        assert(size <= (size_t)(arena->end - arena->ptr));
    }
    void *ptr = arena->ptr;
    arena->ptr = ZENDS_ALIGN_UP_PTR(arena->ptr + size, ARENA_ALIGNMENT);
    assert(arena->ptr <= arena->end);
    assert(ptr == ZENDS_ALIGN_DOWN_PTR(ptr, ARENA_ALIGNMENT));
    return ptr;
}

void arena_free(Arena *arena) {
    for (char **it = arena->blocks; it != buf_end(arena->blocks); it++) {
        free(*it);
    }
    buf_free(arena->blocks);
}


// Hash functions

uint64_t hash_uint64(uint64_t x) {
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

uint64_t hash_ptr(const void *ptr) {
    return hash_uint64((uintptr_t)ptr);
}

uint64_t hash_mix(uint64_t x, uint64_t y) {
    x ^= y;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

uint64_t hash_bytes(const void *ptr, size_t len) {
    uint64_t x = 0xcbf29ce484222325;
    const char *buf = (const char *)ptr;
    for (size_t i = 0; i < len; i++) {
        x ^= buf[i];
        x *= 0x100000001b3;
        x ^= x >> 32;
    }
    return x;
}

// Hash Map

void map_put_uint64_from_uint64(Map *map, uint64_t key, uint64_t val);
uint64_t map_get_uint64_from_uint64(Map *map, uint64_t key) {
    if (map->len == 0) {
        return 0;
    }
    assert(ZENDS_IS_POW2(map->cap));
    size_t i = (size_t)hash_uint64(key);
    assert(map->len < map->cap);
    for (;;) {
        i &= map->cap - 1;
        if (map->keys[i] == key) {
            return map->vals[i];
        } else if (!map->keys[i]) {
            return 0;
        }
        i++;
    }
    return 0;
}

void map_grow(Map *map, size_t new_cap) {
    new_cap = ZENDS_CLAMP_MIN(new_cap, 16);
    Map new_map = {
        .keys = ZENDS_CALLOC(new_cap, sizeof(uint64_t)),
        .vals = ZENDS_MALLOC(new_cap * sizeof(uint64_t)),
        .cap = new_cap,
    };
    for (size_t i = 0; i < map->cap; i++) {
        if (map->keys[i]) {
            map_put_uint64_from_uint64(&new_map, map->keys[i], map->vals[i]);
        }
    }
    ZENDS_FREE((void *)map->keys);
    ZENDS_FREE(map->vals);
    *map = new_map;
}

void map_put_uint64_from_uint64(Map *map, uint64_t key, uint64_t val) {
    assert(key);
    if (!val) {
        return;
    }
    if (2*map->len >= map->cap) {
        map_grow(map, 2*map->cap);
    }
    assert(2*map->len < map->cap);
    assert(ZENDS_IS_POW2(map->cap));
    size_t i = (size_t)hash_uint64(key);
    for (;;) {
        i &= map->cap - 1;
        if (!map->keys[i]) {
            map->len++;
            map->keys[i] = key;
            map->vals[i] = val;
            return;
        } else if (map->keys[i] == key) {
            map->vals[i] = val;
            return;
        }
        i++;
    }
}

void *map_get(Map *map, const void *key) {
    return (void *)(uintptr_t)map_get_uint64_from_uint64(map, (uint64_t)(uintptr_t)key);
}

void map_put(Map *map, const void *key, void *val) {
    map_put_uint64_from_uint64(map, (uint64_t)(uintptr_t)key, (uint64_t)(uintptr_t)val);
}

void *map_get_from_uint64(Map *map, uint64_t key) {
    return (void *)(uintptr_t)map_get_uint64_from_uint64(map, key);
}

void map_put_from_uint64(Map *map, uint64_t key, void *val) {
    map_put_uint64_from_uint64(map, key, (uint64_t)(uintptr_t)val);
}

uint64_t map_get_uint64(Map *map, void *key) {
    return map_get_uint64_from_uint64(map, (uint64_t)(uintptr_t)key);
}

void map_put_uint64(Map *map, void *key, uint64_t val) {
    map_put_uint64_from_uint64(map, (uint64_t)(uintptr_t)key, val);
}

// String interning

typedef struct Intern {
    size_t len;
    struct Intern *next;
    char str[];
} Intern;

Arena intern_arena;
Map interns;
size_t intern_memory_usage;

const char *str_intern_range(const char *start, const char *end) {
    size_t len = end - start;
    uint64_t hash = hash_bytes(start, len);
    uint64_t key = hash ? hash : 1;
    Intern *intern = map_get_from_uint64(&interns, key);
    for (Intern *it = intern; it; it = it->next) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }
    Intern *new_intern = arena_alloc(&intern_arena, offsetof(Intern, str) + len + 1);
    new_intern->len = len;
    new_intern->next = intern;
    memcpy(new_intern->str, start, len);
    new_intern->str[len] = 0;
    map_put_from_uint64(&interns, key, new_intern);
    intern_memory_usage += sizeof(Intern) + len + 1 + 16; /* 16 is estimate of hash table cost */
    return new_intern->str;
}

const char *str_intern(const char *str) {
    return str_intern_range(str, str + strlen(str));
}


#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Tim Wright
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
