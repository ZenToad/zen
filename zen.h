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
typedef size_t    usize;
typedef ptrdiff_t isize;
#endif



// NOTE(tim): Easier to search for
#define cast(Type) (Type)


// NOTE(tim): Because a signed sizeof is more useful
#define zen_sizeof(x) cast(isize)(sizeof(x))


#define zdebug(M, ...) fprintf(stdout, "DEBUG %s:%d: " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)


#define zout(M, ...) fprintf(stdout, M "\n", ##__VA_ARGS__)
#define zfout(v) fprintf(stdout, #v ": %.4f\n", v)
#define ziout(v) fprintf(stdout, #v ": %d\n", v)


#ifndef zen_inline
	#if defined(_MSC_VER)
		#if _MSC_VER < 1300
		#define zen_inline
		#else
		#define zen_inline __forceinline
		#endif
	#else
		#define zen_inline __attribute__ ((__always_inline__))
	#endif
#endif

#ifndef zen_offset_of
#define zen_offset_of(Type, element) ((isize)&(((Type *)0)->element))
#endif

#ifdef ZEN_H_STATIC
#define ZENHDEF static
#else 
#define ZENHDEF extern
#endif


#ifdef __cplusplus
}
#endif


#endif // ZEN_H_INCLUDE

#ifdef ZEN_H_IMPLEMENTATION


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
