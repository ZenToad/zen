/* zen_squtil.h - v0.42 - Squirrel Lang wrapper -https://github.com/ZenToad/zen

   Do this:
      #define SQUTIL_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SQUTIL_IMPLEMENTATION
   #include "zen_squtil.h"

	 zlib license:
	 Full license at bottom of file.

*/

#ifndef SQUTIL_H
#define SQUTIL_H

#ifdef SQUTIL_STATIC
#define SQUTIL_DEF static
#else
#define SQUTIL_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <squirrel.h>

// definitions go here...
SQUTIL_DEF SQRESULT set_table_int(HSQUIRRELVM v, const char* table, const char* key, SQInteger value);
SQUTIL_DEF SQRESULT get_string(HSQUIRRELVM v, const char* name, const SQChar** string);
SQUTIL_DEF SQRESULT get_int( HSQUIRRELVM v, const char* name, SQInteger* integer );
SQUTIL_DEF SQRESULT get_var( HSQUIRRELVM v, const char* name );
SQUTIL_DEF void printfunc( HSQUIRRELVM v, const SQChar *s, ... );
SQUTIL_DEF void errorfunc( HSQUIRRELVM v, const SQChar *s, ... );
SQUTIL_DEF void print_stack( HSQUIRRELVM v );
SQUTIL_DEF void register_global_func( HSQUIRRELVM v, SQFUNCTION f, const char *fname );
SQUTIL_DEF SQRESULT register_global_variable( HSQUIRRELVM v, const char *fname, SQInteger value );
SQUTIL_DEF void call_func( HSQUIRRELVM v, const char* func );

#ifdef __cplusplus
}
#endif

#endif // end SQUTIL_H

#ifdef SQUTIL_IMPLEMENTATION

#include <stdarg.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <stdio.h>

SQUTIL_DEF SQRESULT register_global_variable( HSQUIRRELVM v, const char *name, SQInteger value ) {
	sq_pushstring( v, _SC( name ), -1 );
	sq_pushinteger( v, value );
	return sq_newslot( v, -3, SQFalse );
}

SQUTIL_DEF SQRESULT set_table_int(HSQUIRRELVM v, const char* table, const char* key, SQInteger value) {
	SQInteger top = sq_gettop(v); //saves the stack size before the call
	sq_pushroottable(v);
	SQRESULT result = get_var(v, table);
	if( SQ_SUCCEEDED(result)) {
		sq_pushstring(v, _SC(key), -1);
		sq_pushinteger(v, value);
		result = sq_newslot(v, -3, SQFalse);
	}
	sq_settop(v, top);
	return result;
}

SQUTIL_DEF SQRESULT get_string( HSQUIRRELVM v, const char* name, const SQChar** string ) {
	SQInteger top = sq_gettop(v); //saves the stack size before the call
	sq_pushstring( v, _SC( name ), -1 );
	SQRESULT result = sq_get( v, -2 );
	if( SQ_SUCCEEDED( result ) ) {
		sq_getstring( v, -1, string );
	}
	sq_settop( v, top );
	return result;
}

SQUTIL_DEF SQRESULT get_int( HSQUIRRELVM v, const char* name, SQInteger* integer ) {
	SQInteger top = sq_gettop(v); //saves the stack size before the call
	sq_pushstring( v, _SC( name ), -1 );
	SQRESULT result = sq_get( v, -2 );
	if( SQ_SUCCEEDED( result ) ) {
		sq_getinteger( v, -1, integer );
	}
	sq_settop( v, top );
	return result;
}

SQUTIL_DEF void register_global_func( HSQUIRRELVM v, SQFUNCTION f, const char *fname ) {
	sq_pushstring(v,fname,-1);
	sq_newclosure(v,f,0); //create a new function
	sq_newslot(v,-3,SQFalse);
}

SQUTIL_DEF SQRESULT get_var( HSQUIRRELVM v, const char* name ) {
	sq_pushstring( v, _SC(name), -1 );
	return sq_get( v, -2 );
}

SQUTIL_DEF void printfunc( HSQUIRRELVM v, const SQChar *s, ... ) {
	va_list vl;
	va_start(vl, s);
	vfprintf(stdout, s, vl);
	va_end(vl);
}

SQUTIL_DEF void errorfunc( HSQUIRRELVM v, const SQChar *s, ... ) {
	va_list vl;
	va_start(vl, s);
	vfprintf(stderr, s, vl);
	va_end(vl);
}

SQUTIL_DEF void call_func( HSQUIRRELVM v, const char* func ) {
	if (SQ_SUCCEEDED(get_var(v, func))) {
		sq_pushroottable(v);
		sq_call(v, 1, SQFalse, SQTrue);// leave closure on stack
		sq_pop(v, 1); // pop Coroutine and process
	}
}

static void print_integer( HSQUIRRELVM v, SQInteger top, SQInteger index ) {
	SQInteger value;
	sq_getinteger( v, index, &value );
	printf("[%lld,%lld] %lld\n", top, index, value);
}

static void print_float( HSQUIRRELVM v, SQInteger top, SQInteger index ) {
	SQFloat value;
	sq_getfloat( v, index, &value );
	printf("[%lld,%lld] %f\n", top, index, value);
}

static void print_string( HSQUIRRELVM v, SQInteger top, SQInteger index ) {
	const SQChar* value;
	sq_getstring( v, index, &value );
	printf("[%lld,%lld] %s\n", top, index, value);
}

static void print_bool( HSQUIRRELVM v, SQInteger top, SQInteger index ) {
	SQBool value;
	sq_getbool( v, index, &value );
	printf("[%lld,%lld] %lld\n", top, index, value);
}

static void print_stack_value( HSQUIRRELVM v, SQObjectType type, SQInteger top, SQInteger index ) {
	switch( type ) {
		case OT_NULL: printf("[%lld,%lld] NULL\n", (index - top - 1), index); break;
		case OT_INTEGER: print_integer( v, (index - top - 1), index ); break;
		case OT_FLOAT: print_float( v, (index - top - 1), index ); break;
		case OT_STRING: print_string( v, (index - top - 1), index ); break;
		case OT_TABLE: printf("[%lld,%lld] TABLE\n", (index - top - 1), index); break;
		case OT_ARRAY: printf("[%lld,%lld] ARRAY\n", (index - top - 1), index); break;
		case OT_USERDATA: printf("[%lld,%lld] USERDATA\n", (index - top - 1), index); break;
		case OT_CLOSURE: printf("[%lld,%lld] CLOSURE\n", (index - top - 1), index); break;
		case OT_NATIVECLOSURE: printf("[%lld,%lld] NATIVECLOSURE\n", (index - top - 1), index); break;
		case OT_GENERATOR: printf("[%lld,%lld] GENERATOR\n", (index - top - 1), index); break;
		case OT_USERPOINTER: printf("[%lld,%lld] USERPOINTER\n", (index - top - 1), index); break;
		case OT_BOOL: print_bool( v, top, (index - top - 1) ); break;
		case OT_INSTANCE: printf("[%lld,%lld] INSTANCE\n", (index - top - 1), index); break;
		case OT_CLASS: printf("[%lld,%lld] CLASS\n", (index - top - 1), index); break;
		case OT_WEAKREF: printf("[%lld,%lld] WEAKREF\n", (index - top - 1), index); break;
		default: printf("[%lld,%lld] UNKNOWN\n", (index - top - 1), index); break;
	}
}

SQUTIL_DEF void print_stack(HSQUIRRELVM v) {

	SQInteger top = sq_gettop( v );
	printf("TOP:  %lld\n", top);
	for( int i = top; i > 0; --i ) {
		SQObjectType type = sq_gettype( v, i );
		print_stack_value( v, type, top, i  );
	}
	printf("\n");
}

#endif // end SQUTIL_IMPLEMENTATION

/*
  zlib license:

  Copyright (c) 2017 Timothy Wright https://github.com/ZenToad

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
