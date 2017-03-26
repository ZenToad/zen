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

#include <stdint.h>
#include <stddef.h>

typedef int8_t  	i8;
typedef int16_t 	i16;
typedef int32_t 	i32;
typedef int64_t 	i64;

typedef uint8_t  	u8;
typedef uint16_t 	u16;
typedef uint32_t 	u32;
typedef uint64_t 	u64;

typedef i8  b8;
typedef i16 b16;
typedef i32 b32; // NOTE(tim): Prefer this!!!

typedef float 		f32;
typedef double 	f64;

typedef size_t    usize;
typedef ptrdiff_t isize;


// NOTE(tim): Easier to search for
#ifndef cast
#define cast(Type) (Type)
#endif


// NOTE(tim): Because a signed sizeof is more useful
#ifndef zen_sizeof
#define zen_sizeof(x) cast(isize)(sizeof(x))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define zdebug(M, ...) fprintf(stdout, "DEBUG %s:%d: " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)


#define zout(M, ...) fprintf(stdout, M "\n", ##__VA_ARGS__)


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
