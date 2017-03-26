/* zen_math.h - v0.02 - public domain utility -https://github.com/ZenToad/zen
                                     no warranty implied; use at your own risk

   Do this:
      #define ZEN_MATH_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define ZEN_MATH_IMPLEMENTATION
   #include "zen_math.h"

	Full license at bottom of file.	

*/

#ifndef ZEN_H_INCLUDE
#define ZEN_H_INCLUDE


#ifdef __cplusplus
extern "C" {
#endif


#ifdef ZEN_MATH_STATIC
#define ZMATH static
#else 
#define ZMATH extern
#endif


#ifdef __cplusplus
}
#endif


#endif // ZEN_MATH_INCLUDE

#ifdef ZEN_H_IMPLEMENTATION

#endif // ZEN_MATH_IMPLEMENTATION


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
