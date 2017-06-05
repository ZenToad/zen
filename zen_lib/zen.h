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
   #include "zen_lib/zen.h"

	Full license at bottom of file.	

*/

#ifndef ZEN_H_INCLUDE
#define ZEN_H_INCLUDE

//@TODO: need some kind of cross platform file i/o
//maybe look at SDL?
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
typedef intptr_t  psize;


#define cast(Type) (Type)
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

#define zen_min(a,b) (a) <= (b) ? (a) : (b)
#define zen_max(a,b) (a) >= (b) ? (a) : (b)
#define zen_at_least(a,b) zen_max(a,b)
#define zen_at_most(a,b) zen_min(a,b)
#define zen_clamp(a,min,max) zen_at_least(min,zen_at_most(max,a))
#define zen_clamp_01(a) zen_at_least(0.0,zen_at_most(1.0,a))


#define zdebug(M, ...) fprintf(stdout, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
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


// The static array needs a 
// max, count, and cmp.  Do we really
// need the cmp?  Could just pass in a func ptr... cmp(void *a, void *b);
// Can we do the same tricks...
//

#if 0

	so how can this work...

#define	MakeStatic(prefix, type, size, data)
	typedef struct ##prefix_StaticArray_##type {
		int n = 0;
		int m = size;
		##data a[size];
	} __##prefix_StaticArray_##type;

#define static_array(prefix,type) __##prefix_StaticArray_##type.a;
#define static_add(a,v)
	int n = ((int *)(a) - 2)[0];
	int m = ((int *)(a) - 2)[1];
	GB_ASSERT(m > n);
	a[((int *)(a) - 2)[0]++] = v;

	
	MakeStatic(arr, int, 64, int);
	int *static = static_array(arr,int);

	static_add(static, 32);

	can this work?  Would be cool;

#endif

#if 0 
template <class K, class V> 
class HashmapNode_t {
	K key;
	V value;
	uint32_t hash;

public:
	HashmapNode_t(K key, V value) : key(key), value(value) {  }
	virtual ~HashmapNode_t() {  }

};


template <class K, class V> 
class Hashmap_t {

	int num_buckets = 0;

	HashmapNode_t<K,V> **buckets;
	int (*compare)(K a, K b);

	uint32_t hash(K key);
	int getNode(HashmapNode_t<K,V> *bucket, uint32_t hash);

public:

	Hashmap_t(int buckets, int (*compare)(K, V));
	virtual ~Hashmap_t();

	V get(K key);
	void set(K key, V value);
	V remove(K key);
	void traverse(int (*traverse)(HashmapNode_t<K,V> *));

};


template <class K, class V>
Hashmap_t<K,V>::Hashmap_t(int num_buckets, int (*compare)(K,V)) {
	this->num_buckets = num_buckets;
	this->compare = compare;
	for (int i = 0; i < num_buckets; ++i)
		 stb_arr_push(this->buckets, NULL);
}

template <class K, class V>
Hashmap_t<K,V>::~Hashmap_t() {
	for (int i = 0; i < stb_arr_len(buckets); ++i) {
		auto *bucket = buckets[i];
		for (int j = 0; j < stb_arr_len(bucket); ++j) {
			auto *node = bucket[j];
			GB_ASSERT_NOT_NULL(node);
			free(node);
		}
		stb_arr_free(bucket);
	}
	stb_arr_free(buckets);
}

template <class K, class V>
int Hashmap_t<K,V>::getNode(HashmapNode_t<K,V> *bucket, uint32_t hash) {
	for (int i = 0; i < stb_arr_len(bucket); i++) {
		auto *node = bucket[i];
		if (node && node->hash == hash) {
			return i;
		}
	}
	return -1;
}

template <class K, class V>
V Hashmap_t<K,V>::get(K key) {

	uint32_t hash = hash(key);
	int bucket_index = hash % num_buckets;
	GB_ASSERT_MSG(bucket_index >= 0, "Invalid bucket_index found: %d", bucket_index);

	auto *bucket = buckets[bucket_index];
	if (bucket == NULL) return NULL; // not found

	int i = getNode(hash, bucket);
	if (i == -1) return NULL;

	auto *node = bucket[i];
	GB_ASSERT_MSG(node != NULL, "Failed to get node from bucket when it should exist.");

	return node->value;

}

template <class K, class V>
void Hashmap_t<K,V>::set(K key, V value) {
	uint32_t hash = hash(key);
	int bucket_index = hash % num_buckets;
	GB_ASSERT_MSG(bucket_index >= 0, "Invalid bucket_index found: %d", bucket_index);

	auto *bucket = buckets[bucket_index];
	for (int i = 0; i < stb_arr_len(bucket); ++i) {
		auto *node = bucket[i];
		if (node && node->hash == hash) {
			node->value = value;
			return;
		}
	}

	HashmapNode_t<K,V> *node = new HashmapNode_t<K,V>(key, value);
	GB_ASSERT_NOT_NULL(node);

	stb_arr_push(buckets[bucket_index], node);
}

template <class K, class V>
V Hashmap_t<K,V>::remove(K key) {

	uint32_t hash = hash(key);
	int bucket_index = hash % num_buckets;
	GB_ASSERT_MSG(bucket_index >= 0, "Invalid bucket_index found: %d", bucket_index);

	auto *bucket = buckets[bucket_index];
	if (bucket == NULL) return NULL; // not found

	int i = getNode(hash, bucket);
	if (i == -1) return NULL;

	auto *node = bucket[i];
	GB_ASSERT_NOT_NULL(node);
	V value = node->value;

	if (i != stb_arr_lastn(bucket)) {
		stb_arr_fastdelete(bucket, i);
	}
	delete node;

	return value;

}

template <class K, class V>
void Hashmap_t<K,V>::traverse(int (*traverse)(HashmapNode_t<K,V> *)) {

	for (int i = 0; i < stb_arr_len(buckets); i++) {
		auto *bucket = buckets[i];
		if (bucket) {
			for (int j = 0; j < stb_arr_len(bucket); j++) {
				auto *node = bucket[j];
				int rc = traverse_cb(node);
				if (rc != 0)
					return;
			}
		}
	}

}

#endif

#if 0
// Static array ideas
// TODO really easy static array list
// needs an array, size, count(n), and cmp() func
typedef struct StaticArray {
	enum{ MAX=512 };
	int a[MAX];
	int n = 0;
	bool (*cmp)(int a, int b);
} StaticArray;


void add(StaticArray *arr, int v) {
	GB_ASSERT(arr->MAX > arr->n);
	arr->a[arr->n++] = v;
}

void remove_index(StaticArray *arr, int i) {
	if (i < 0 || i >= arr->n)
		return; // not in list
	if (i < arr->n - 1) // wasn't last
		arr->a[i] = arr->a[arr->n-1];
	arr->n--;
}	

int find_index(StaticArray *arr, int v) {
	for (int i = 0; i < arr->n; ++i) {
		if(arr->cmp(arr->a[i],v))
			return i;
	}
	return -1;
}

void remove(StaticArray *arr, int v) {
	remove_index(arr, find_index(arr, v));
}

#endif



#if 0
// Simple hashmap ideas
#define DEFAULT_NUMBER_OF_BUCKETS 100

typedef int (*Hashmap_compare) (const char *a, const char *b);
typedef uint32_t(*Hashmap_hash) (const char *key);
typedef struct HashmapNode {

    const char *key;
    gbString data;
    uint32_t hash;

} HashmapNode;

typedef HashmapNode * HashmapBucket;

typedef struct Hashmap {

    HashmapBucket **buckets;
    Hashmap_compare compare;
    Hashmap_hash hash;

} Hashmap;


typedef int (*Hashmap_traverse_cb) (HashmapNode * node);

/** 
 * Simple Bob Jenkins's hash algorithm taken from the
 * wikipedia description.
 */
uint32_t default_hash(const char *a) {
    size_t len = strlen(a);
    uint32_t hash = 0;
    uint32_t i = 0;

    for (hash = i = 0; i < len; ++i) {
        hash += a[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}


int default_compare(const char *a, const char *b) {
    return strcmp(a, b);
}


Hashmap *Hashmap_create(Hashmap_compare compare, Hashmap_hash hash) {

    Hashmap *map = ZEN_CALLOC(Hashmap, 1);
	 GB_ASSERT_NOT_NULL(map);

    map->compare = compare == NULL ? default_compare : compare;
    map->hash = hash == NULL ? default_hash : hash;

	 for (int i = 0; i < DEFAULT_NUMBER_OF_BUCKETS; ++i)
		 stb_arr_push(map->buckets, NULL);

    return map;

}


void Hashmap_destroy(Hashmap * map) {

	if (map) {
		for (int i = 0; i < stb_arr_len(map->buckets); ++i) {
			HashmapBucket *bucket = map->buckets[i];
			for (int j = 0; j < stb_arr_len(bucket); ++j) {
				HashmapNode *node = bucket[j];
				GB_ASSERT_NOT_NULL(node);
				free(node);
			}
			stb_arr_free(bucket);
		}
		stb_arr_free(map->buckets);
	}
	free(map);

}


static inline int Hashmap_get_node(Hashmap *map, uint32_t hash, HashmapBucket *bucket, const char *key) {

	for (int i = 0; i < stb_arr_len(bucket); i++) {
		HashmapNode *node = bucket[i];
		if (node && node->hash == hash) {
			return i;
		}
	}

	return -1;
}


gbString Hashmap_get(Hashmap *map, const char *key) {

	uint32_t hash = map->hash(key);
	int bucket_index = hash % DEFAULT_NUMBER_OF_BUCKETS;
	GB_ASSERT_MSG(bucket_index >= 0, "Invalid bucket_index found: %d", bucket_index);

	HashmapBucket *bucket = map->buckets[bucket_index];
	if (bucket == NULL) return NULL; // not found

	int i = Hashmap_get_node(map, hash, bucket, key);
	if (i == -1) return NULL;

	HashmapNode *node = bucket[i];
	GB_ASSERT_MSG(node != NULL, "Failed to get node from bucket when it should exist.");

	return node->data;

}


int Hashmap_traverse(Hashmap * map, Hashmap_traverse_cb traverse_cb) {

	for (int i = 0; i < stb_arr_len(map->buckets); i++) {
		HashmapBucket *bucket = map->buckets[i];
		if (bucket) {
			for (int j = 0; j < stb_arr_len(bucket); j++) {
				HashmapNode *node = bucket[j];
				int rc = traverse_cb(node);
				if (rc != 0)
					return rc;
			}
		}
	}

	return 0;
}


gbString Hashmap_delete(Hashmap *map, const char *key) {

	uint32_t hash = map->hash(key);
	int bucket_index = hash % DEFAULT_NUMBER_OF_BUCKETS;
	GB_ASSERT_MSG(bucket_index >= 0, "Invalid bucket_index found: %d", bucket_index);

	HashmapBucket *bucket = map->buckets[bucket_index];
	if (bucket == NULL) return NULL; // not found

	int i = Hashmap_get_node(map, hash, bucket, key);
	if (i == -1) return NULL;

	HashmapNode *node = bucket[i];
	GB_ASSERT_NOT_NULL(node);
	gbString data = node->data;

	if (i != stb_arr_lastn(bucket)) {
		stb_arr_fastdelete(bucket, i);
	}
	free(node);

	return data;
}

int count_new = 0;

void Hashmap_set(Hashmap *map, const char *key, gbString data) {

	uint32_t hash = map->hash(key);
	int bucket_index = hash % DEFAULT_NUMBER_OF_BUCKETS;
	GB_ASSERT_MSG(bucket_index >= 0, "Invalid bucket_index found: %d", bucket_index);

	HashmapBucket *bucket = map->buckets[bucket_index];
	for (int i = 0; i < stb_arr_len(bucket); ++i) {
		HashmapNode *node = bucket[i];
		if (node && node->hash == hash) {
			node->data = data;
			return;
		}
	}

	HashmapNode *node = ZEN_CALLOC(HashmapNode, 1);
	GB_ASSERT_NOT_NULL(node);

	node->key = key;
	node->data = data;
	node->hash = hash;

	stb_arr_push(map->buckets[bucket_index], node);
	count_new++;

}

#endif



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
static void stb_swap(void *p, void *q, size_t sz)
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
