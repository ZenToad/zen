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
typedef intptr_t  psize;

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
//                                stb_arr
//
//  An stb_arr is directly useable as a pointer (use the actual type in your
//  definition), but when it resizes, it returns a new pointer and you can't
//  use the old one, so you have to be careful to copy-in-out as necessary.
//
//  Use a NULL pointer as a 0-length array.
//
//     float *my_array = NULL, *temp;
//
//     // add elements on the end one at a time
//     stb_arr_push(my_array, 0.0f);
//     stb_arr_push(my_array, 1.0f);
//     stb_arr_push(my_array, 2.0f);
//
//     assert(my_array[1] == 2.0f);
//
//     // add an uninitialized element at the end, then assign it
//     *stb_arr_add(my_array) = 3.0f;
//
//     // add three uninitialized elements at the end
//     temp = stb_arr_addn(my_array,3);
//     temp[0] = 4.0f;
//     temp[1] = 5.0f;
//     temp[2] = 6.0f;
//
//     assert(my_array[5] == 5.0f);
//
//     // remove the last one
//     stb_arr_pop(my_array);
//
//     assert(stb_arr_len(my_array) == 6);


// simple functions written on top of other functions
//
ZENHDEF void stb_swap(void *p, void *q, size_t sz);

#define stb_arr_empty(a)       (  stb_arr_len(a) == 0 )
#define stb_arr_add(a)         (  stb_arr_addn((a),1) )
#define stb_arr_push(a,v)      ( *stb_arr_add(a)=(v)  )


typedef struct {
   int len, limit;
   unsigned int signature;
} stb__arr;


#define stb_arr_signature      0x51bada7b  // ends with 0123 in decimal


// access the header block stored before the data
#define stb_arrhead(a)         /*lint --e(826)*/ (((stb__arr *) (psize)(a)) - 1)
#define stb_arrhead2(a)        /*lint --e(826)*/ (((stb__arr *) (psize)(a)) - 1)


#ifdef STB_DEBUG
#define stb_arr_check(a)       assert(!a || stb_arrhead(a)->signature == stb_arr_signature)
#define stb_arr_check2(a)      assert(!a || stb_arrhead2(a)->signature == stb_arr_signature)
#else
#define stb_arr_check(a)       ((void) 0)
#define stb_arr_check2(a)      ((void) 0)
#endif


// ARRAY LENGTH

// get the array length; special case if pointer is NULL
#define stb_arr_len(a)         (a ? stb_arrhead(a)->len : 0)
#define stb_arr_len2(a)        ((stb__arr *) (a) ? stb_arrhead2(a)->len : 0)
#define stb_arr_lastn(a)       (stb_arr_len(a)-1)

// check whether a given index is valid -- tests 0 <= i < stb_arr_len(a) 
#define stb_arr_valid(a,i)     (a ? (int) (i) < stb_arrhead(a)->len : 0)

// change the array length so is is exactly N entries long, creating
// uninitialized entries as needed
#define stb_arr_setlen(a,n)  \
            (stb__arr_setlen((void **) &(a), sizeof(a[0]), (n)))

// change the array length so that N is a valid index (that is, so
// it is at least N entries long), creating uninitialized entries as needed
#define stb_arr_makevalid(a,n)  \
            (stb_arr_len(a) < (n)+1 ? stb_arr_setlen(a,(n)+1),(a) : (a))

// remove the last element of the array, returning it
#define stb_arr_pop(a)         ((stb_arr_check(a), (a))[--stb_arrhead(a)->len])

// access the last element in the array
#define stb_arr_last(a)        ((stb_arr_check(a), (a))[stb_arr_len(a)-1])

// is iterator at end of list?
#define stb_arr_end(a,i)       ((i) >= &(a)[stb_arr_len(a)])

// (internal) change the allocated length of the array
#define stb_arr__grow(a,n)     (stb_arr_check(a), stb_arrhead(a)->len += (n))

// add N new unitialized elements to the end of the array
#define stb_arr__addn(a,n)     /*lint --e(826)*/ \
                               ((stb_arr_len(a)+(n) > stb_arrcurmax(a))      \
                                 ? (stb__arr_addlen((void **) &(a),sizeof(*a),(n)),0) \
                                 : ((stb_arr__grow(a,n), 0)))

// add N new unitialized elements to the end of the array, and return
// a pointer to the first new one
#define stb_arr_addn(a,n)      (stb_arr__addn((a),n),(a)+stb_arr_len(a)-(n))

// add N new uninitialized elements starting at index 'i'
#define stb_arr_insertn(a,i,n) (stb__arr_insertn((void **) &(a), sizeof(*a), i, n))

// insert an element at i
#define stb_arr_insert(a,i,v)  (stb__arr_insertn((void **) &(a), sizeof(*a), i, 1), ((a)[i] = v))

// delete N elements from the middle starting at index 'i'
#define stb_arr_deleten(a,i,n) (stb__arr_deleten((void **) &(a), sizeof(*a), i, n))

// delete the i'th element
#define stb_arr_delete(a,i)   stb_arr_deleten(a,i,1)

// delete the i'th element, swapping down from the end
#define stb_arr_fastdelete(a,i)  \
   (stb_swap(&a[i], &a[stb_arrhead(a)->len-1], sizeof(*a)), stb_arr_pop(a))


// ARRAY STORAGE

// get the array maximum storage; special case if NULL
#define stb_arrcurmax(a)       (a ? stb_arrhead(a)->limit : 0)
#define stb_arrcurmax2(a)      (a ? stb_arrhead2(a)->limit : 0)

// set the maxlength of the array to n in anticipation of further growth
#define stb_arr_setsize(a,n)   (stb_arr_check(a), stb__arr_setsize((void **) &(a),sizeof((a)[0]),n))

// make sure maxlength is large enough for at least N new allocations
#define stb_arr_atleast(a,n)   (stb_arr_len(a)+(n) > stb_arrcurmax(a)      \
                                 ? stb_arr_setsize((a), (n)) : 0)

// make a copy of a given array (copies contents via 'memcpy'!)
#define stb_arr_copy(a)        stb__arr_copy(a, sizeof((a)[0]))

// compute the storage needed to store all the elements of the array
#define stb_arr_storage(a)     (stb_arr_len(a) * sizeof((a)[0]))

#define stb_arr_for(v,arr)     for((v)=(arr); (v) < (arr)+stb_arr_len(arr); ++(v))


ZENHDEF void stb_arr_free_(void **p);
ZENHDEF void *stb__arr_copy_(void *p, int elem_size);
ZENHDEF void stb__arr_setsize_(void **p, int size, int limit);
ZENHDEF void stb__arr_setlen_(void **p, int size, int newlen);
ZENHDEF void stb__arr_addlen_(void **p, int size, int addlen);
ZENHDEF void stb__arr_deleten_(void **p, int size, int loc, int n);
ZENHDEF void stb__arr_insertn_(void **p, int size, int loc, int n);


#define stb_arr_free(p)            stb_arr_free_((void **) &(p))
#define stb__arr_copy              stb__arr_copy_


#ifndef STB_MALLOC_WRAPPER
  #define stb__arr_setsize         stb__arr_setsize_
  #define stb__arr_setlen          stb__arr_setlen_
  #define stb__arr_addlen          stb__arr_addlen_
  #define stb__arr_deleten         stb__arr_deleten_
  #define stb__arr_insertn         stb__arr_insertn_
#else
  #define stb__arr_addlen(p,s,n)    stb__arr_addlen_(p,s,n,__FILE__,__LINE__)
  #define stb__arr_setlen(p,s,n)    stb__arr_setlen_(p,s,n,__FILE__,__LINE__)
  #define stb__arr_setsize(p,s,n)   stb__arr_setsize_(p,s,n,__FILE__,__LINE__)
  #define stb__arr_deleten(p,s,i,n) stb__arr_deleten_(p,s,i,n,__FILE__,__LINE__)
  #define stb__arr_insertn(p,s,i,n) stb__arr_insertn_(p,s,i,n,__FILE__,__LINE__)
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
#define sb_pop		stb_sb_pop
#endif

#define stb_sb_free(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define stb_sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define stb_sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])
#define stb_sb_pop(a)          ((a)[--stb__sbn(a)])

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      (*((void **)&(a)) = stb__sbgrowf((a), (n), sizeof(*(a))))



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


//------------------------------------------ 
//
// stb_arr implementation
//
//


void * stb__arr_copy_(void *p, int elem_size) {
   stb__arr *q;
   if (p == NULL) return p;
   q = (stb__arr *) malloc(sizeof(*q) + elem_size * stb_arrhead2(p)->limit);
   stb_arr_check2(p);
   memcpy(q, stb_arrhead2(p), sizeof(*q) + elem_size * stb_arrhead2(p)->len);
   return q+1;
}


void stb_arr_free_(void **pp) {
   void *p = *pp;
   stb_arr_check2(p);
   if (p) {
      stb__arr *q = stb_arrhead2(p);
		free(q);
   }
   *pp = NULL;
}


static void stb__arrsize_(void **pp, int size, int limit, int len) {
   void *p = *pp;
   stb__arr *a;
   stb_arr_check2(p);
   if (p == NULL) {
      if (len == 0 && size == 0) return;
      a = (stb__arr *) malloc(sizeof(*a) + size*limit);
      a->limit = limit;
      a->len   = len;
      a->signature = stb_arr_signature;
   } else {
      a = stb_arrhead2(p);
      a->len = len;
      if (a->limit < limit) {
         void *p;
         if (a->limit >= 4 && limit < a->limit * 2)
            limit = a->limit * 2;
            p = realloc(a, sizeof(*a) + limit*size);
         if (p) {
            a = (stb__arr *) p;
            a->limit = limit;
         } else {
            // throw an error!
         }
      }
   }
   a->len   = zen_min(a->len, a->limit);
   *pp = a+1;
}


void stb__arr_setsize_(void **pp, int size, int limit) {
   void *p = *pp;
   stb_arr_check2(p);
   stb__arrsize_(pp, size, limit, stb_arr_len2(p));
}


void stb__arr_setlen_(void **pp, int size, int newlen) {
   void *p = *pp;
   stb_arr_check2(p);
   if (stb_arrcurmax2(p) < newlen || p == NULL) {
      stb__arrsize_(pp, size, newlen, newlen);
   } else {
      stb_arrhead2(p)->len = newlen;
   }
}


void stb__arr_addlen_(void **p, int size, int addlen) {
   stb__arr_setlen_(p, size, stb_arr_len2(*p) + addlen);
}


void stb__arr_insertn_(void **pp, int size, int i, int n) {
   void *p = *pp;
   if (n) {
      int z;

      if (p == NULL) {
         stb__arr_addlen_(pp, size, n);
         return;
      }

      z = stb_arr_len2(p);
      stb__arr_addlen_(&p, size, n);
      memmove((char *) p + (i+n)*size, (char *) p + i*size, size * (z-i));
   }
   *pp = p;
}


void stb__arr_deleten_(void **pp, int size, int i, int n) {
   void *p = *pp;
   if (n) {
      memmove((char *) p + i*size, (char *) p + (i+n)*size, size * (stb_arr_len2(p)-(i+n)));
      stb_arrhead2(p)->len -= n;
   }
   *pp = p;
}


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
