#if defined(ZEN_LIB_IMPLEMENTATION)

#define ZEN_H_IMPLEMENTATION
#define ZEN_ARR_IMPLEMENTATION
#define ZEN_MATH_IMPLEMENTATION
#define ZEN_HASHMAP_IMPLEMENTATION

#endif


#if !defined(__ZEN_LIB_H__)
#define __ZEN_LIB_H__


#if !defined(ZEN_LIB_NO_INCLUDES)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#endif


#include "zen_lib/zen.h"
#include "zen_lib/zen_arr.h"
#include "zen_lib/zen_math.h"
#include "zen_lib/zen_hashmap.h"


#endif //__ZEN_LIB_H__
