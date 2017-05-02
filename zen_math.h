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

#ifndef __ZEN_MATN_H__
#define __ZEN_MATN_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifndef zout
#include "zen.h"
#endif

#ifndef uint8
#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
#endif

#ifndef cosf
#include <math.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#define ZM_DEG2RAD(d) ((d) * M_PI / 180.0)
#define ZM_RAD2DEG(r) ((r) * 180.0 / M_PI)
#define ZM_DEG2RADF(d) (cast(float)ZM_DEG2RAD((d)))
#define ZM_RAD2DEGF(r) (cast(float)ZM_RAD2DEG((d)))


#ifdef ZEN_MATH_STATIC
#define ZMATHDEF static
#else 
#define ZMATHDEF extern
#endif


typedef union Matrix2x2_t {
	float m[4];
} Matrix2x2_t;

typedef union Matrix3x3_t {
	float m[9];
} Matrix3x3_t;

typedef union Matrix4x4_t {
	float m[16];
} Matrix4x4_t;

typedef union Vector2_t {
	struct { float x, y; };
	float e[2];
} Vector2_t;

typedef union Vector3_t {
	struct { float x, y, z; };
	float e[3];
} Vector3_t;

typedef union Vector4_t {
	struct { float x, y, z, w; };
	float e[4];
} Vector4_t;

typedef union Color_t {
	struct{ uint8 r, g, b, a; };
	uint32 rgba;
	uint8 e[4];
} Color_t;

typedef union Colorf_t {
	struct{ float r, g, b, a; };
	float e[4];
} Colorf_t;

typedef struct Transform2d_t {

	Vector2_t position;
	Vector2_t scale;
	float rotation;

} Transform2d_t;

ZMATHDEF Matrix2x2_t Matrix2x2();
ZMATHDEF Matrix3x3_t Matrix3x3();
ZMATHDEF Matrix4x4_t Matrix4x4();

ZMATHDEF Vector2_t Vector2(float x, float y);
ZMATHDEF Vector3_t Vector3(float x, float y, float z);
ZMATHDEF Vector4_t Vector4(float x, float y, float z, float w);

ZMATHDEF Color_t Color(uint8 r, uint8 g, uint8 b, uint8 a);
ZMATHDEF Colorf_t Colorf(float r, float g, float b, float a = 1.0f);

ZMATHDEF Transform2d_t Transform2d(Vector2_t position, float rotation, Vector2_t scale);

ZMATHDEF Vector2_t add_vec2(Vector2_t a, Vector2_t b);
ZMATHDEF Vector2_t sub_vec2(Vector2_t a, Vector2_t b);
ZMATHDEF Vector2_t mul_vec2(Vector2_t v, float s);
ZMATHDEF float dot_vec2(Vector2_t a, Vector2_t b);
ZMATHDEF float len_vec2(Vector2_t a);
ZMATHDEF float len_sqr_vec2(Vector2_t a);
ZMATHDEF Vector2_t norm_vec2(Vector2_t a);
ZMATHDEF Vector2_t lerp_vec2(Vector2_t a, Vector2_t b, float t);

ZMATHDEF Vector3_t add_vec3(Vector3_t a, Vector3_t b);
ZMATHDEF Vector3_t sub_vec3(Vector3_t a, Vector3_t b);
ZMATHDEF Vector3_t mul_vec3(Vector3_t v, float s);
ZMATHDEF float dot_vec3(Vector3_t a, Vector3_t b);

ZMATHDEF Vector4_t add_vec4(Vector4_t a, Vector4_t b);
ZMATHDEF Vector4_t sub_vec4(Vector4_t a, Vector4_t b);
ZMATHDEF Vector4_t mul_vec4(Vector4_t v, float s);
ZMATHDEF float dot_vec4(Vector4_t a, Vector4_t b);

ZMATHDEF Matrix2x2_t mul_mat2x2(Matrix2x2_t a, Matrix2x2_t b);
ZMATHDEF Matrix3x3_t mul_mat3x3(Matrix3x3_t a, Matrix3x3_t b);
ZMATHDEF Matrix4x4_t mul_mat4x4(Matrix4x4_t a, Matrix4x4_t b);

ZMATHDEF Matrix3x3_t trans_mat3x3(Matrix3x3_t m, Vector2_t v);
ZMATHDEF Matrix3x3_t rot_mat3x3(Matrix3x3_t m, float rad);
ZMATHDEF Matrix3x3_t scale_mat3x3(Matrix3x3_t m, Vector2_t v);

ZMATHDEF Matrix4x4_t trans_mat4x4(Matrix4x4_t m, Vector3_t v);
ZMATHDEF Matrix4x4_t rotz_mat4x4(Matrix4x4_t m, float rad);
ZMATHDEF Matrix4x4_t scale_mat4x4(Matrix4x4_t m, Vector3_t v);

ZMATHDEF void print4x4(Matrix4x4_t m);
ZMATHDEF void print_v3(Vector3_t v);
ZMATHDEF void print3x3(Matrix3x3_t m);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

zen_inline Vector2_t operator+(Vector2_t a) {return a;}
zen_inline Vector2_t operator+(Vector2_t a, Vector2_t b) {return Vector2(a.x + b.x, a.y + b.y);}

zen_inline Vector2_t operator-(Vector2_t a) {return Vector2(-a.x, -a.y);}
zen_inline Vector2_t operator-(Vector2_t a, Vector2_t b) {return Vector2(a.x - b.x, a.y - b.y);}

zen_inline Vector2_t operator*(Vector2_t a, float s) {return Vector2(a.x * s, a.y * s);}
zen_inline Vector2_t operator*(float s, Vector2_t a) {return Vector2(a.x * s, a.y * s);}

zen_inline Vector2_t operator/(Vector2_t a, float s) {return Vector2(a.x / s, a.y / s);}

zen_inline Vector2_t& operator+=(Vector2_t &a, Vector2_t b) {return a = a + b;}
zen_inline Vector2_t& operator-=(Vector2_t &a, Vector2_t b) {return a = a - b;}
zen_inline Vector2_t& operator*=(Vector2_t &a, float s) {return a = a * s;}
zen_inline Vector2_t& operator/=(Vector2_t &a, float s) {return a = a / s;}


zen_inline float len(Vector2_t a) {return len_vec2(a);}
zen_inline float len_sqr(Vector2_t a) {return len_sqr_vec2(a);}
zen_inline float dot(Vector2_t a, Vector2_t b) {return dot_vec2(a, b);}
zen_inline Vector2_t norm(Vector2_t a) {return norm_vec2(a);}
zen_inline Vector2_t lerp(Vector2_t a, Vector2_t b, float t) {return (1.0f - t) * a + t * b;}


zen_inline Matrix2x2_t operator*(Matrix2x2_t a, Matrix2x2_t b) {
	Matrix2x2_t out;
	for (int32 i = 0; i < 2; ++i) {
		for (int32 j = 0; j < 2; ++j) {
			float f = 0.f;
			for (int32 k = 0; k < 2; ++k) {
				f += a.m[i*2+k] * b.m[k*2+j];
			}
			out.m[i*2+j] = f;
		}
	}
	return out;
}

zen_inline Matrix3x3_t operator*(Matrix3x3_t a, Matrix3x3_t b) {
	Matrix3x3_t out;
	for (int32 i = 0; i < 3; ++i) {
		for (int32 j = 0; j < 3; ++j) {
			float f = 0.f;
			for (int32 k = 0; k < 3; ++k) {
				f += a.m[i*3+k] * b.m[k*3+j];
			}
			out.m[i*3+j] = f;
		}
	}
	return out;
}

zen_inline Vector3_t operator*(Matrix3x3_t m, Vector3_t v) {
	Vector3_t out;
	for (int32 i = 0; i < 3; ++i) {
		float f = 0.f;
		for (int32 j = 0; j < 3; ++j) {
			f += m.m[j*3+i] * v.e[j];
		}
		out.e[i] = f;
	}
	return out;
}

zen_inline Matrix3x3_t translate(Vector2_t v) {
	Matrix3x3_t trans = Matrix3x3();
	trans.m[6] = v.x;
	trans.m[7] = v.y;
	return trans;
}

zen_inline Matrix3x3_t rotate2D(float rad) {
	Matrix3x3_t rot = Matrix3x3();
	float c = cosf(rad);
	float s = sinf(rad);
	rot.m[0] = c;
	rot.m[1] = s;
	rot.m[3] = -s;
	rot.m[4] = c;
	return rot;
}

zen_inline Matrix3x3_t scale(Vector2_t v) {
	Matrix3x3_t scale = Matrix3x3();
	scale.m[0] = v.x;
	scale.m[4] = v.y;
	return scale;
}

zen_inline void print(Matrix3x3_t m) {print3x3(m);}

#endif

#endif // __ZEN_MATN_H__


//------------------------------------------ 
//
//
// IMPLEMENTATION
//
//
//------------------------------------------


#ifdef ZEN_MATH_IMPLEMENTATION

Color_t COLOR_WHITE = Color(255, 255, 255, 255);
Color_t COLOR_SILVER = Color(191, 191, 191, 255);
Color_t COLOR_GRAY = Color(128, 128, 128, 255);
Color_t COLOR_BLACK = Color(0, 0, 0, 255);
Color_t COLOR_RED = Color(255, 0, 0, 255);
Color_t COLOR_MAROON = Color(128, 0, 0, 255);
Color_t COLOR_YELLOW = Color(255, 255, 0, 255);
Color_t COLOR_OLIVE = Color(128, 128, 0, 255);
Color_t COLOR_LIME = Color(0, 255, 0, 255);
Color_t COLOR_GREEN = Color(0, 128, 0, 255);
Color_t COLOR_AQUA = Color(0, 255, 255, 255);
Color_t COLOR_TEAL = Color(0, 128, 128, 255);
Color_t COLOR_BLUE = Color(0, 0, 255, 255);
Color_t COLOR_NAVY = Color(0, 0, 128, 255);
Color_t COLOR_FUCHSIA = Color(255, 0, 255, 255);
Color_t COLOR_PURPLE = Color(128, 0, 128, 255);

ZMATHDEF Matrix2x2_t Matrix2x2() {
	Matrix2x2_t m;
	m.m[0] = 1.f; m.m[1] = 0.f;
	m.m[2] = 0.f; m.m[3] = 1.f;
	return m;
}

ZMATHDEF Matrix3x3_t Matrix3x3() {
	Matrix3x3_t m;
	for (int32 i = 0; i < 9; ++i)
		m.m[i] = 0.f;
	m.m[0] = 1.f;
	m.m[4] = 1.f;
	m.m[8] = 1.f;
	return m;
}

ZMATHDEF Matrix4x4_t Matrix4x4() {
	Matrix4x4_t m;
	for (int32 i = 0; i < 16; ++i)
		m.m[i] = 0.0f;

	m.m[0] = 1.0f;
	m.m[5] = 1.0f;
	m.m[10] = 1.0f;
	m.m[15] = 1.0f;
	return m;
}

ZMATHDEF Vector2_t Vector2(float x, float y) {
	Vector2_t v;
	v.x = x;
	v.y = y;
	return v;
}

ZMATHDEF Vector3_t Vector3(float x, float y, float z) {
	Vector3_t v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

ZMATHDEF Vector4_t Vector4(float x, float y, float z, float w) {
	Vector4_t v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

ZMATHDEF Color_t Color(uint8 r, uint8 g, uint8 b, uint8 a) {
	Color_t c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return c;
}

ZMATHDEF Colorf_t Colorf(float r, float g, float b, float a) {
	Colorf_t c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return c;
}

ZMATHDEF Transform2d_t Transform2d(Vector2_t position, float rotation, Vector2_t scale) {
	Transform2d_t t;
	t.position = position;
	t.rotation = rotation;
	t.scale = scale;
	return t;
}

ZMATHDEF Vector2_t add_vec2(Vector2_t a, Vector2_t b) {
	return Vector2(a.x + b.x, a.y + b.y);
}

ZMATHDEF Vector2_t sub_vec2(Vector2_t a, Vector2_t b) {
	return Vector2(a.x - b.x, a.y - b.y);
}

ZMATHDEF Vector2_t mul_vec2(Vector2_t v, float s) {
	return Vector2(v.x * s, v.y * s);
}

ZMATHDEF float dot_vec2(Vector2_t a, Vector2_t b) {
	return a.x * b.x + a.y * b.y;
}

ZMATHDEF float len_vec2(Vector2_t a) {
	return sqrt(a.x*a.x + a.y*a.y);
}

ZMATHDEF float len_sqr_vec2(Vector2_t a) {
	return a.x*a.x + a.y*a.y;
}

ZMATHDEF Vector2_t norm_vec2(Vector2_t a) {
	float len = sqrt(a.x*a.x + a.y*a.y);
	if (len == 0.0) return Vector2(0, 0);
	return mul_vec2(a, 1.0f / len);
}

ZMATHDEF Vector2_t lerp_vec2(Vector2_t a, Vector2_t b, float t) {
	Vector2_t result;
	result.x = (1.0f - t) * a.x + t * b.x;
	result.y = (1.0f - t) * a.y + t * b.y;
	return result;
}

ZMATHDEF Vector3_t add_vec3(Vector3_t a, Vector3_t b) {
	return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

ZMATHDEF Vector3_t sub_vec3(Vector3_t a, Vector3_t b) {
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

ZMATHDEF Vector3_t mul_vec3(Vector3_t v, float s) {
	return Vector3(v.x * s, v.y * s, v.z * s);
}

ZMATHDEF float dot_vec3(Vector3_t a, Vector3_t b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

ZMATHDEF Vector4_t add_vec4(Vector4_t a, Vector4_t b) {
	return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

ZMATHDEF Vector4_t sub_vec4(Vector4_t a, Vector4_t b) {
	return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

ZMATHDEF Vector4_t mul_vec4(Vector4_t v, float s) {
	return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
}

ZMATHDEF float dot_vec4(Vector4_t a, Vector4_t b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

ZMATHDEF Matrix2x2_t mul_mat2x2(Matrix2x2_t a, Matrix2x2_t b) {
	Matrix2x2_t out;
	for (int32 i = 0; i < 2; ++i) {
		for (int32 j = 0; j < 2; ++j) {
			float f = 0.f;
			for (int32 k = 0; k < 2; ++k) {
				f += a.m[i*2+k] * b.m[k*2+j];
			}
			out.m[i*2+j] = f;
		}
	}
	return out;
}

ZMATHDEF Matrix3x3_t mul_mat3x3(Matrix3x3_t a, Matrix3x3_t b) {
	Matrix3x3_t out;
	for (int32 i = 0; i < 3; ++i) {
		for (int32 j = 0; j < 3; ++j) {
			float f = 0.f;
			for (int32 k = 0; k < 3; ++k) {
				f += a.m[i*3+k] * b.m[k*3+j];
			}
			out.m[i*3+j] = f;
		}
	}
	return out;
}

ZMATHDEF Matrix4x4_t mul_mat4x4(Matrix4x4_t a, Matrix4x4_t b) {
	Matrix4x4_t out;
	for (int32 i = 0; i < 4; ++i) {
		for (int32 j = 0; j < 4; ++j) {
			float f = 0.f;
			for (int32 k = 0; k < 4; ++k) {
				f += a.m[i*4+k] * b.m[k*4+j];
			}
			out.m[i*4+j] = f;
		}
	}
	return out;
}

ZMATHDEF Vector3_t mul_mat3x3_vec3(Matrix3x3_t m, Vector3_t v) {
	Vector3_t out;
	for (int32 i = 0; i < 3; ++i) {
		float f = 0.f;
		for (int32 j = 0; j < 3; ++j) {
			f += m.m[j*3+i] * v.e[j];
		}
		out.e[i] = f;
	}
	return out;
}

ZMATHDEF Vector4_t mul_mat4x4_vec4(Matrix4x4_t m, Vector4_t v) {
	Vector4_t out;
	for (int32 i = 0; i < 4; ++i) {
		float f = 0.f;
		for (int32 j = 0; j < 4; ++j) {
			f += m.m[j*4+i] * v.e[j];
		}
		out.e[i] = f;
	}
	return out;
}

ZMATHDEF Matrix3x3_t trans_mat3x3(Matrix3x3_t m, Vector2_t v) {
	Matrix3x3_t trans = Matrix3x3();
	trans.m[6] = v.x;
	trans.m[7] = v.y;
	return mul_mat3x3(m, trans);
}

ZMATHDEF Matrix3x3_t rot_mat3x3(Matrix3x3_t m, float rad) {
	Matrix3x3_t rot = Matrix3x3();
	float c = cosf(rad);
	float s = sinf(rad);
	rot.m[0] = c;
	rot.m[1] = s;
	rot.m[3] = -s;
	rot.m[4] = c;
	return mul_mat3x3(m, rot);
}

ZMATHDEF Matrix3x3_t scale_mat3x3(Matrix3x3_t m, Vector2_t v) {
	Matrix3x3_t scale = Matrix3x3();
	scale.m[0] = v.x;
	scale.m[4] = v.y;
	return mul_mat3x3(m, scale);
}

ZMATHDEF Matrix4x4_t trans_mat4x4(Matrix4x4_t m, Vector3_t v) {
	Matrix4x4_t trans = Matrix4x4();
	trans.m[12] = v.x;
	trans.m[13] = v.y;
	trans.m[14] = v.z;
	return mul_mat4x4(m, trans);
}

ZMATHDEF Matrix4x4_t rotz_mat4x4(Matrix4x4_t m, float rad) {

	Matrix4x4_t rot = Matrix4x4();
	float c = cosf(rad);
	float s = sinf(rad);
	
	rot.m[0] = c;
	rot.m[1] = s;
	rot.m[4] = -s;
	rot.m[5] = c;

	return mul_mat4x4(m, rot);
}

ZMATHDEF Matrix4x4_t scale_mat4x4(Matrix4x4_t m, Vector3_t v) {
	Matrix4x4_t scale = Matrix4x4();
	scale.m[0] = v.x;
	scale.m[5] = v.y;
	scale.m[10] = v.z;
	return mul_mat4x4(m, scale);
}

ZMATHDEF void print_v2(Vector2_t v) {
	zout("[%.4f, %.4f]\n", v.e[0], v.e[1]);
}

ZMATHDEF void print_v3(Vector3_t v) {
	zout("[%.4f, %.4f, %.4f]\n", v.e[0], v.e[1], v.e[2]);
}

ZMATHDEF void print_v4(Vector4_t v) {
	zout("[%.4f, %.4f, %.4f, %.4f]\n", v.e[0], v.e[1], v.e[2], v.e[2]);
}

ZMATHDEF void print4x4(Matrix4x4_t m) {
	for (int32 i = 0; i < 4; ++i) {
		zout("[%.4f, %.4f, %.4f, %.4f]", m.m[i*4+0], m.m[i*4+1], m.m[i*4+2], m.m[i*4+3]);
	}
	zout("");
}

ZMATHDEF void print3x3(Matrix3x3_t m) {
	for (int32 i = 0; i < 3; ++i) {
		zout("[%.4f, %.4f, %.4f]", m.m[i*3+0], m.m[i*3+1], m.m[i*3+2]);
	}
	zout("");
}

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
