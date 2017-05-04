/* zen.h - v0.42 - public domain utility -https://github.com/ZenToad/zen
                                     no warranty implied; use at your own risk

   Do this:
      #define ZEN_H_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define ZEN_H_IMPLEMENTATION
   #include "zen.h"

	Full license at bottom of file.	

*/

#ifndef ZEN_H_INCLUDE
#define ZEN_H_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ZEN_LIB_DEV)
#include <stdio.h>
#endif
  
#if !defined(int32_t)
#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32 b32;
#endif

#if !defined(size_t) || !defined(ptrdiff_t)
#include <stddef.h>
#endif

typedef size_t    usize;
typedef ptrdiff_t isize;

// NOTE(tim): Easier to search for
#define cast(Type) (Type)

// NOTE(tim): Because a signed sizeof is more useful
#define zen_sizeof(x) cast(isize)(sizeof(x))


#if !defined(ZEN_MALLOC)
#define ZEN_MALLOC(T) (T *)malloc(zen_sizeof(T))
#endif

#if !defined(ZEN_MALLOC_ARRAY)
#define ZEN_MALLOC_ARRAY(T,count) (T *)malloc(zen_sizeof(T) * (count))
#endif

#if !defined(ZEN_CALLOC)
#define ZEN_CALLOC(T,count) (T*)calloc((count), zen_sizeof(T))
#endif

#define zdebug(M, ...) fprintf(stdout, "DEBUG %s:%d: " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)


#define zout(M, ...) fprintf(stdout, M "\n", ##__VA_ARGS__)
#define zfout(v) fprintf(stdout, #v ": %.4f\n", v)
#define ziout(v) fprintf(stdout, #v ": %d\n", v)


#define zen_offset(x) ((const void*) (x))
#define zen_offset_of(Type, element) ((isize)&(((Type *)0)->element))


#ifndef zen_inline
	#if defined(_MSC_VER)
		#if _MSC_VER < 1300
		#define zen_inline
		#else
		#define zen_inline __forceinline
		#endif
	#else
		#define zen_inline __attribute__ ((__always_inline__)) inline
	#endif
#endif



////////////////////////////////////////////////////////////////////////////////////
//
// stretchy_buffer.h stb libraries
//
//


#ifndef NO_STRETCHY_BUFFER_SHORT_NAMES
#define sb_free   stb_sb_free
#define sb_push   stb_sb_push
#define sb_count  stb_sb_count
#define sb_add    stb_sb_add
#define sb_last   stb_sb_last
#endif

#define stb_sb_free(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define stb_sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define stb_sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      (*((void **)&(a)) = stb__sbgrowf((a), (n), sizeof(*(a))))


////////////////////////////////////////////////////////////////
//
// Debug Ginger Bill's gb.h
//
//


#ifndef GB_DEBUG_TRAP
	#if defined(_MSC_VER)
	 	#if _MSC_VER < 1300
		#define GB_DEBUG_TRAP() __asm int 3 /* Trap to debugger! */
		#else
		#define GB_DEBUG_TRAP() __debugbreak()
		#endif
	#else
		#define GB_DEBUG_TRAP() raise(SIGTRAP)
	#endif
#endif

#ifndef GB_ASSERT_MSG
#define GB_ASSERT_MSG(cond, msg, ...) do { \
	if (!(cond)) { \
		gb_assert_handler(#cond, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
		GB_DEBUG_TRAP(); \
	} \
} while (0)
#endif

#ifndef GB_ASSERT
#define GB_ASSERT(cond) GB_ASSERT_MSG(cond, NULL)
#endif

#ifndef GB_ASSERT_NOT_NULL
#define GB_ASSERT_NOT_NULL(ptr) GB_ASSERT_MSG((ptr) != NULL, #ptr " must not be NULL")
#endif

// NOTE(bill): Things that shouldn't happen with a message!
#ifndef GB_PANIC
#define GB_PANIC(msg, ...) GB_ASSERT_MSG(0, msg, ##__VA_ARGS__)
#endif

#ifdef ZEN_H_STATIC
#define ZENHDEF static
#else 
#define ZENHDEF extern
#endif


ZENHDEF void gb_assert_handler(char const *condition, char const *file, int line, char const *msg, ...);

////////////////////////////////////////////////////////////////
//
// DArray - Learn C the hard way
//
//
//


typedef struct DArray {
    int end;
    int max;
    size_t element_size;
    size_t expand_rate;
    void **contents;
} DArray;

DArray *DArray_create(size_t element_size, size_t initial_max);

void DArray_destroy(DArray * array);

void DArray_clear(DArray * array);

int DArray_expand(DArray * array);

int DArray_contract(DArray * array);

int DArray_push(DArray * array, void *el);

void *DArray_pop(DArray * array);

void DArray_clear_destroy(DArray * array);

#define DArray_last(A) ((A)->contents[(A)->end - 1])
#define DArray_first(A) ((A)->contents[0])
#define DArray_end(A) ((A)->end)
#define DArray_count(A) DArray_end(A)
#define DArray_max(A) ((A)->max)

#define DArray_free(E) free((E))


////////////////////////////////////////////////////////////////////////////////////
//
// lcthw Hashmap
//
//


#define DEFAULT_NUMBER_OF_BUCKETS 100

typedef int (*Hashmap_compare) (void *a, void *b);
typedef uint32_t(*Hashmap_hash) (void *key);

typedef struct Hashmap {
    DArray *buckets;
    Hashmap_compare compare;
    Hashmap_hash hash;
} Hashmap;

typedef struct HashmapNode {
    void *key;
    void *data;
    uint32_t hash;
} HashmapNode;

typedef int (*Hashmap_traverse_cb) (HashmapNode * node);

Hashmap *Hashmap_create(Hashmap_compare compare, Hashmap_hash);
void Hashmap_destroy(Hashmap * map);

int Hashmap_set(Hashmap * map, void *key, void *data);
void *Hashmap_get(Hashmap * map, void *key);

int Hashmap_traverse(Hashmap * map, Hashmap_traverse_cb traverse_cb);

void *Hashmap_delete(Hashmap * map, void *key);


//////////////////////////////////////////////////////////////////////////////
//
//               Random Numbers via Meresenne Twister or LCG
//
// stb.h

ZENHDEF unsigned long stb_srandLCG(unsigned long seed);
ZENHDEF unsigned long stb_randLCG(void);
ZENHDEF double        stb_frandLCG(void);

ZENHDEF void          stb_srand(unsigned long seed);
ZENHDEF unsigned long stb_rand(void);
ZENHDEF double        stb_frand(void);
ZENHDEF void          stb_shuffle(void *p, size_t n, size_t sz,
                                        unsigned long seed);
ZENHDEF void stb_reverse(void *p, size_t n, size_t sz);

ZENHDEF unsigned long stb_randLCG_explicit(unsigned long seed);

#define stb_rand_define(x,y)                                         \
                                                                     \
   unsigned long x(void)                                             \
   {                                                                 \
      static unsigned long stb__rand = y;                            \
      stb__rand = stb__rand * 2147001325 + 715136305; /* BCPL */     \
      return 0x31415926 ^ ((stb__rand >> 16) + (stb__rand << 16));   \
   }


#ifdef __cplusplus
}
#endif


#endif // ZEN_H_INCLUDE


//------------------------------------------ 
//
// Implementation
//
//------------------------------------------ 
#ifdef ZEN_H_IMPLEMENTATION

#define DEFAULT_EXPAND_RATE 300

static inline void DArray_set(DArray * array, int i, void *el) {
    GB_ASSERT_MSG(i < array->max, "darray attempt to set past max");
    if (i > array->end)
        array->end = i;
    array->contents[i] = el;
}

static inline void *DArray_get(DArray * array, int i) {
    GB_ASSERT_MSG(i < array->max, "darray attempt to get past max");
    return array->contents[i];
}

static inline void *DArray_remove(DArray * array, int i)
{
    void *el = array->contents[i];

    array->contents[i] = NULL;

    return el;
}

static inline void *DArray_new(DArray * array)
{
    GB_ASSERT_MSG(array->element_size > 0,
            "Can't use DArray_new on 0 size darrays.");

    return calloc(1, array->element_size);

}

DArray *DArray_create(size_t element_size, size_t initial_max)
{
    DArray *array = ZEN_MALLOC(DArray);
    GB_ASSERT_NOT_NULL(array);
    array->max = initial_max;
    GB_ASSERT_MSG(array->max > 0, "You must set an initial_max > 0.");

    array->contents = ZEN_CALLOC(void*, initial_max);
    GB_ASSERT_NOT_NULL(array->contents);

    array->end = 0;
    array->element_size = element_size;
    array->expand_rate = DEFAULT_EXPAND_RATE;

    return array;

}

void DArray_clear(DArray * array)
{
    int i = 0;
    if (array->element_size > 0) {
        for (i = 0; i < array->max; i++) {
            if (array->contents[i] != NULL) {
                free(array->contents[i]);
            }
        }
    }
}

static inline int DArray_resize(DArray * array, size_t newsize)
{
    array->max = newsize;
    GB_ASSERT_MSG(array->max > 0, "The newsize must be > 0.");

    void *contents = realloc(
            array->contents, array->max * sizeof(void *));
    // check contents and assume realloc doesn't harm the original on error

    GB_ASSERT_NOT_NULL(contents);

    array->contents = (void **)contents;

    return 0;
}

int DArray_expand(DArray * array)
{
    size_t old_max = array->max;
    GB_ASSERT_MSG(DArray_resize(array, array->max + array->expand_rate) == 0,
            "Failed to expand array to new size: %d",
            array->max + (int)array->expand_rate);

    memset(array->contents + old_max, 0, array->expand_rate + 1);
    return 0;

}

int DArray_contract(DArray * array)
{
    int new_size = array->end < (int)array->expand_rate ? 
            (int)array->expand_rate : array->end;

    return DArray_resize(array, new_size + 1);
}

void DArray_destroy(DArray * array)
{
    if (array) {
        if (array->contents)
            free(array->contents);
        free(array);
    }
}

void DArray_clear_destroy(DArray * array)
{
    DArray_clear(array);
    DArray_destroy(array);
}

int DArray_push(DArray * array, void *el)
{
    array->contents[array->end] = el;
    array->end++;

    if (DArray_end(array) >= DArray_max(array)) {
        return DArray_expand(array);
    } else {
        return 0;
    }
}

void *DArray_pop(DArray * array)
{
    GB_ASSERT_MSG(array->end - 1 >= 0, "Attempt to pop from empty array.");

    void *el = DArray_remove(array, array->end - 1);
    array->end--;

    if (DArray_end(array) > (int)array->expand_rate
            && DArray_end(array) % array->expand_rate) {
        DArray_contract(array);
    }

    return el;
}

static int default_compare(void *a, void *b) {
	return gb_strings_are_equal((gbString) a, (gbString) b);
}

/** 
 * Simple Bob Jenkins's hash algorithm taken from the
 * wikipedia description.
 */
static uint32_t default_hash(void *a)
{
	gbString key = (gbString)a;
	gbUsize len = gb_string_length(key);
    uint32_t hash = 0;
    uint32_t i = 0;

    for (hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

Hashmap *Hashmap_create(Hashmap_compare compare, Hashmap_hash hash)
{
    Hashmap *map = (Hashmap *)calloc(1, sizeof(Hashmap));
    GB_ASSERT_NOT_NULL(map);

    map->compare = compare == NULL ? default_compare : compare;
    map->hash = hash == NULL ? default_hash : hash;
    map->buckets = DArray_create(
            sizeof(DArray *), DEFAULT_NUMBER_OF_BUCKETS);
    map->buckets->end = map->buckets->max;	// fake out expanding it
    GB_ASSERT_NOT_NULL(map->buckets);

    return map;

}

void Hashmap_destroy(Hashmap * map)
{
    int i = 0;
    int j = 0;

    if (map) {
        if (map->buckets) {
            for (i = 0; i < DArray_count(map->buckets); i++) {
                DArray *bucket = (DArray *)DArray_get(map->buckets, i);
                if (bucket) {
                    for (j = 0; j < DArray_count(bucket); j++) {
                        free(DArray_get(bucket, j));
                    }
                    DArray_destroy(bucket);
                }
            }
            DArray_destroy(map->buckets);
        }

        free(map);
    }
}

static inline HashmapNode *Hashmap_node_create(int hash, void *key,
        void *data)
{
    HashmapNode *node = (HashmapNode *)calloc(1, sizeof(HashmapNode));
    GB_ASSERT_NOT_NULL(node);

    node->key = key;
    node->data = data;
    node->hash = hash;

    return node;

}

static inline DArray *Hashmap_find_bucket(Hashmap * map, void *key,
        int create,
        uint32_t * hash_out)
{
    uint32_t hash = map->hash(key);
    int bucket_n = hash % DEFAULT_NUMBER_OF_BUCKETS;
    GB_ASSERT_MSG(bucket_n >= 0, "Invalid bucket found: %d", bucket_n);
    // store it for the return so the caller can use it
    *hash_out = hash;

    DArray *bucket = (DArray *)DArray_get(map->buckets, bucket_n);

    if (!bucket && create) {
        // new bucket, set it up
        bucket = DArray_create(
                sizeof(void *), DEFAULT_NUMBER_OF_BUCKETS);
        GB_ASSERT_NOT_NULL(bucket);
        DArray_set(map->buckets, bucket_n, bucket);
    }

    return bucket;

}

int Hashmap_set(Hashmap * map, void *key, void *data)
{
    uint32_t hash = 0;
    DArray *bucket = Hashmap_find_bucket(map, key, 1, &hash);
    GB_ASSERT_MSG(bucket, "Error can't create bucket.");

    HashmapNode *node = Hashmap_node_create(hash, key, data);
    GB_ASSERT_NOT_NULL(node);

    DArray_push(bucket, node);

    return 0;

}

static inline int Hashmap_get_node(Hashmap * map, uint32_t hash,
        DArray * bucket, void *key)
{
    int i = 0;

    for (i = 0; i < DArray_end(bucket); i++) {
        zdebug("TRY: %d", i);
        HashmapNode *node = (HashmapNode *)DArray_get(bucket, i);
        if (node->hash == hash && map->compare(node->key, key) == 0) {
            return i;
        }
    }

    return -1;
}

void *Hashmap_get(Hashmap * map, void *key)
{
    uint32_t hash = 0;
    DArray *bucket = Hashmap_find_bucket(map, key, 0, &hash);
    if (!bucket) return NULL;

    int i = Hashmap_get_node(map, hash, bucket, key);
    if (i == -1) return NULL;

    HashmapNode *node = (HashmapNode *)DArray_get(bucket, i);
    GB_ASSERT_MSG(node != NULL,
            "Failed to get node from bucket when it should exist.");

    return node->data;

}

int Hashmap_traverse(Hashmap * map, Hashmap_traverse_cb traverse_cb)
{
    int i = 0;
    int j = 0;
    int rc = 0;

    for (i = 0; i < DArray_count(map->buckets); i++) {
        DArray *bucket = (DArray *)DArray_get(map->buckets, i);
        if (bucket) {
            for (j = 0; j < DArray_count(bucket); j++) {
                HashmapNode *node = (HashmapNode *)DArray_get(bucket, j);
                rc = traverse_cb(node);
                if (rc != 0)
                    return rc;
            }
        }
    }

    return 0;
}

void *Hashmap_delete(Hashmap * map, void *key)
{
    uint32_t hash = 0;
    DArray *bucket = Hashmap_find_bucket(map, key, 0, &hash);
    if (!bucket)
        return NULL;

    int i = Hashmap_get_node(map, hash, bucket, key);
    if (i == -1)
        return NULL;

    HashmapNode *node = (HashmapNode *)DArray_get(bucket, i);
    void *data = node->data;
    free(node);

    HashmapNode *ending = (HashmapNode *)DArray_pop(bucket);

    if (ending != node) {
        // alright looks like it's not the last one, swap it
        DArray_set(bucket, i, ending);
    }

    return data;
}


////////////////////////////////////////////////////////////////////////////////////
//
// stretchy_buffer.h stb libraries
//
//


static void * stb__sbgrowf(void *arr, int increment, int itemsize) {
   int dbl_cur = arr ? 2*stb__sbm(arr) : 0;
   int min_needed = stb_sb_count(arr) + increment;
   int m = dbl_cur > min_needed ? dbl_cur : min_needed;
   int *p = (int *) realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int)*2);
   if (p) {
      if (!arr)
         p[1] = 0;
      p[0] = m;
      return p+2;
   } else {
      #ifdef STRETCHY_BUFFER_OUT_OF_MEMORY
      STRETCHY_BUFFER_OUT_OF_MEMORY ;
      #endif
      return (void *) (2*sizeof(int)); // try to force a NULL pointer exception later
   }
}


////////////////////////////////////////////////////////////////
//
// Debug Ginger Bill's gb.h
//
//


void gb_assert_handler(char const *condition, char const *file, int line, char const *msg, ...) {
	fprintf(stderr, "%s:%d: Assert Failure: ", file, line);
	if (condition)
		fprintf(stderr, "`%s` ", condition);
	if (msg) {
		va_list va;
		va_start(va, msg);
		vfprintf(stderr, msg, va);
		va_end(va);
	}
	fprintf(stderr, "\n");
}


//////////////////////////////////////////////////////////////////////////////
//
//               Random Numbers via Meresenne Twister or LCG
//
// stb.h
//

typedef struct { char d[4]; } stb__4;
typedef struct { char d[8]; } stb__8;


// optimize the small cases, though you shouldn't be calling this for those!
void stb_swap(void *p, void *q, size_t sz)
{
   char buffer[256];
   if (p == q) return;
   if (sz == 4) {
      stb__4 temp    = * ( stb__4 *) p;
      * (stb__4 *) p = * ( stb__4 *) q;
      * (stb__4 *) q = temp;
      return;
   } else if (sz == 8) {
      stb__8 temp    = * ( stb__8 *) p;
      * (stb__8 *) p = * ( stb__8 *) q;
      * (stb__8 *) q = temp;
      return;
   }

   while (sz > sizeof(buffer)) {
      stb_swap(p, q, sizeof(buffer));
      p = (char *) p + sizeof(buffer);
      q = (char *) q + sizeof(buffer);
      sz -= sizeof(buffer);
   }

   memcpy(buffer, p     , sz);
   memcpy(p     , q     , sz);
   memcpy(q     , buffer, sz);
}

unsigned long stb_randLCG_explicit(unsigned long seed)
{
   return seed * 2147001325 + 715136305;
}

static unsigned long stb__rand_seed=0;

unsigned long stb_srandLCG(unsigned long seed)
{
   unsigned long previous = stb__rand_seed;
   stb__rand_seed = seed;
   return previous;
}

unsigned long stb_randLCG(void)
{
   stb__rand_seed = stb__rand_seed * 2147001325 + 715136305; // BCPL generator
   // shuffle non-random bits to the middle, and xor to decorrelate with seed
   return 0x31415926 ^ ((stb__rand_seed >> 16) + (stb__rand_seed << 16));
}

double stb_frandLCG(void)
{
   return stb_randLCG() / ((double) (1 << 16) * (1 << 16));
}

void stb_shuffle(void *p, size_t n, size_t sz, unsigned long seed)
{
   char *a;
   unsigned long old_seed = 0;
   size_t i;
   if (seed)
      old_seed = stb_srandLCG(seed);
   a = (char *) p + (n-1) * sz;

   for (i=n; i > 1; --i) {
      int j = stb_randLCG() % i;
      stb_swap(a, (char *) p + j * sz, sz);
      a -= sz;
   }
   if (seed)
      stb_srandLCG(old_seed);
}

void stb_reverse(void *p, size_t n, size_t sz)
{
   size_t i,j = n-1;
   for (i=0; i < j; ++i,--j) {
      stb_swap((char *) p + i * sz, (char *) p + j * sz, sz);
   }
}

// public domain Mersenne Twister by Michael Brundage
#define STB__MT_LEN       624

int stb__mt_index = STB__MT_LEN*sizeof(unsigned long)+1;
unsigned long stb__mt_buffer[STB__MT_LEN];

void stb_srand(unsigned long seed)
{
   int i;
   unsigned long old = stb_srandLCG(seed);
   for (i = 0; i < STB__MT_LEN; i++)
      stb__mt_buffer[i] = stb_randLCG();
   stb_srandLCG(old);
   stb__mt_index = STB__MT_LEN*sizeof(unsigned long);
}

#define STB__MT_IA           397
#define STB__MT_IB           (STB__MT_LEN - STB__MT_IA)
#define STB__UPPER_MASK      0x80000000
#define STB__LOWER_MASK      0x7FFFFFFF
#define STB__MATRIX_A        0x9908B0DF
#define STB__TWIST(b,i,j)    ((b)[i] & STB__UPPER_MASK) | ((b)[j] & STB__LOWER_MASK)
#define STB__MAGIC(s)        (((s)&1)*STB__MATRIX_A)

unsigned long stb_rand()
{
   unsigned long * b = stb__mt_buffer;
   int idx = stb__mt_index;
   unsigned long s,r;
   int i;
	
   if (idx >= STB__MT_LEN*sizeof(unsigned long)) {
      if (idx > STB__MT_LEN*sizeof(unsigned long))
         stb_srand(0);
      idx = 0;
      i = 0;
      for (; i < STB__MT_IB; i++) {
         s = STB__TWIST(b, i, i+1);
         b[i] = b[i + STB__MT_IA] ^ (s >> 1) ^ STB__MAGIC(s);
      }
      for (; i < STB__MT_LEN-1; i++) {
         s = STB__TWIST(b, i, i+1);
         b[i] = b[i - STB__MT_IB] ^ (s >> 1) ^ STB__MAGIC(s);
      }
      
      s = STB__TWIST(b, STB__MT_LEN-1, 0);
      b[STB__MT_LEN-1] = b[STB__MT_IA-1] ^ (s >> 1) ^ STB__MAGIC(s);
   }
   stb__mt_index = idx + sizeof(unsigned long);
   
   r = *(unsigned long *)((unsigned char *)b + idx);
   
   r ^= (r >> 11);
   r ^= (r << 7) & 0x9D2C5680;
   r ^= (r << 15) & 0xEFC60000;
   r ^= (r >> 18);
   
   return r;
}

double stb_frand(void)
{
   return stb_rand() / ((double) (1 << 16) * (1 << 16));
}

#endif // ZEN_IMPLEMENTATION


// Public Domain (www.unlicense.org)
// This is free and unencumbered software released into the public domain.
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
// software, either in source code form or as a compiled binary, for any purpose, 
// commercial or non-commercial, and by any means.
// In jurisdictions that recognize copyright laws, the author or authors of this 
// software dedicate any and all copyright interest in the software to the public 
// domain. We make this dedication for the benefit of the public at large and to 
// the detriment of our heirs and successors. We intend this dedication to be an 
// overt act of relinquishment in perpetuity of all present and future rights to 
// this software under copyright law.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
