#if !defined(__ZEN_GLEZ_H__)
#define __ZEN_GLEZ_H__


#if defined(ZEN_LIB_DEV)
#include "glad/glad.h"
#include "zen/zen_lib/zen.h"
#include "zen/zen_lib/zen_math.h"
#include "zen/zen_lib/zen_hashmap.h"
#endif


#if !defined(ZGLEZ_FONT_WIDTH)
#define ZGLEZ_FONT_WIDTH	2048
#endif
#if !defined(ZGLEZ_FONT_HEIGHT)
#define ZGLEZ_FONT_HEIGHT	2048
#endif
 

#if defined(ZEN_GLEZ_STATIC)
#define ZGLEZDEF static
#else
#define ZGLEZDEF extern
#endif


#if defined(__cplusplus)
extern "C" {
#endif


ZGLEZDEF void zglez_init();
ZGLEZDEF void zglez_quit();
ZGLEZDEF void zglez_flush();
ZGLEZDEF void zglez_projection(Matrix4x4_t m);
ZGLEZDEF void zglez_text_projection(Matrix4x4_t m);


ZGLEZDEF int zglez_load_texture_from_file(const char *name, const char * path_to_file, int *w = 0, int *h = 0, int flip_vertically = 0);
ZGLEZDEF int zglez_load_texture_from_memory(const char *name, char *memory, isize size_in_bytes);
ZGLEZDEF int zglez_unload_texture(const char *name);
ZGLEZDEF void zglez_unload_all_textures();


ZGLEZDEF int zglez_load_font_from_memory(const char *name, unsigned char *memory, isize size_in_bytes, int pixel_size, int padding = 4, int oversample = 1);
ZGLEZDEF int zglez_unload_font(const char *name);
ZGLEZDEF void zglez_unload_all_fonts();


ZGLEZDEF void zglez_draw_point(Vector3_t v, Colorf_t c, float pt_size = 2.0f);
ZGLEZDEF void zglez_draw_line(Vector3_t v0, Vector3_t v1, Colorf_t c);
ZGLEZDEF void zglez_draw_circle(Vector3_t center, float radius, Colorf_t c, int steps = 32);
ZGLEZDEF void zglez_fill_circle(Vector3_t center, float radius, Colorf_t c, int steps = 32, bool blend = true);
ZGLEZDEF void zglez_draw_polygon(Vector3_t *v, Colorf_t c, int count);
ZGLEZDEF void zglez_fill_polygon(Vector3_t *v, Colorf_t c, int count, bool blend = true);
ZGLEZDEF void zglez_texture_quad(const char *name, Vector3_t *v, Vector2_t *t);
ZGLEZDEF void zglez_draw_text(const char *name, const char *string, Vector2_t position, Colorf_t color);



#if defined(__cplusplus)
}
#endif


#endif // __ZEN_GLEZ_H__


//------------------------------------------ 
//
//
// IMPLEMENTATION
//
//
//------------------------------------------ 
#if defined(ZEN_GLEZ_IMPLEMENTATION) || defined(ZEN_LIB_DEV)


#if defined(ZEN_LIB_DEV)
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "glad/glad.h"
#include "stb_image.h"
#include "stb_truetype.h"
#include "zen/zen_lib.h"
#endif


static int32 const zglez_internal_texture_format[4] = {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8};
static int32 const zglez_texture_format[4] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};


#define ZGLEZ_CHECK_ERROR() do { \
	GLenum error_code = glGetError(); \
	if (error_code != GL_NO_ERROR) { \
		GB_PANIC("OpenGL error = %d\n", error_code); \
	} \
} while (0)


typedef struct zglez_texture_t {

	int32 width;
	int32 height;
	int32 channel_count;
	uint32 handle;
	uint32 active_texture;
	uint32 sampler;

} zglez_texture_t;


stb_declare_hash(static, zglez_texture_map, zglez_texmap_, const char*, zglez_texture_t*);
stb_define_hash_vnull(zglez_texture_map, zglez_texmap_, const char*, NULL, NULL, return stb_hash((char *)k);, zglez_texture_t*, NULL); 


typedef struct zglez_font_t {

	stbtt_fontinfo font;
	// @TODO: not sure what this size should really be
	stbtt_packedchar *pdata;

	int32 width;
	int32 height;
	uint32 handle;
	uint32 active_texture;
	uint32 sampler;

} zglez_font_t;


stb_declare_hash(static, zglez_font_map, zglez_fontmap_, const char*, zglez_font_t*);
stb_define_hash_vnull(zglez_font_map, zglez_fontmap_, const char*, NULL, NULL, return stb_hash((char*)k);, zglez_font_t*, NULL); 


typedef struct zglez_points {
 
	enum { max_vertices = 512 };
	Vector3_t vertices[max_vertices];
	Colorf_t colors[max_vertices];
	float sizes[max_vertices];
 
	Matrix4x4_t projection;

	int32 count;
    
	GLuint vao_id;
	GLuint vbo_ids[3];
	GLuint program_id;
	GLint projection_uniform;
	GLint vertex_attribute;
	GLint color_attribute;
	GLint size_attribute;

} zglez_points;
static zglez_points *g_zglez_points = 0;


typedef struct zglez_lines {
 
	enum { max_vertices = 512 * 2 };
	Vector3_t vertices[max_vertices];
	Colorf_t colors[max_vertices];
 
	Matrix4x4_t projection;

	int32 count;
    
	GLuint vao_id;
	GLuint vbo_ids[2];
	GLuint program_id;
	GLint projection_uniform;
	GLint vertex_attribute;
	GLint color_attribute;

} zglez_lines;
static zglez_lines *g_zglez_lines = 0;


typedef struct zglez_triangles {
 
	enum { max_vertices = 512 * 3 };
	Vector3_t vertices[max_vertices];
	Colorf_t colors[max_vertices];
 
	Matrix4x4_t projection;

	int32 count;
    
	GLuint vao_id;
	GLuint vbo_ids[2];
	GLuint program_id;
	GLint projection_uniform;
	GLint vertex_attribute;
	GLint color_attribute;

} zglez_triangles;
static zglez_triangles *g_zglez_triangles = 0;


typedef struct zglez_textures {

	enum {max_vertices = 32 * 6};
	Vector3_t vertices[max_vertices];
	Vector2_t tex_coords[max_vertices];
	int32 count;

	Matrix4x4_t projection;

	zglez_texture_t *current_texture;
	zglez_texture_map *map;

	GLuint vao_id;
	GLuint vbo_ids[2];
	GLuint program_id;
	GLint projection_uniform;
	GLint sampler_location;
	GLint vertex_attribute;
	GLint tex_attribute;

} zglez_textures;
static zglez_textures *g_zglez_textures = 0;


typedef struct zglez_fonts {

	enum {max_vertices = 512 * 6};
	Vector3_t vertices[max_vertices];
	Vector2_t tex_coords[max_vertices];
	Colorf_t colors[max_vertices];
	int32 count;

	Matrix4x4_t projection;

	zglez_font_t *current_font;
	zglez_font_map *map;

	GLuint vao_id;
	GLuint vbo_ids[3];
	GLuint program_id;
	GLint projection_uniform;
	GLint sampler_location;
	GLint vertex_attribute;
	GLint tex_attribute;
	GLint color_attribute;

} zglez_fonts;
static zglez_fonts *g_zglez_fonts = 0;


static void zglez_print_log(GLuint object) {

	GLint log_length = 0;
	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if (glIsProgram(object)) {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else {
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}

	char* log = (char*)malloc(log_length);

	if (glIsShader(object)) {
		glGetShaderInfoLog(object, log_length, NULL, log);
	} else if (glIsProgram(object)) {
		glGetProgramInfoLog(object, log_length, NULL, log);
	}

	fprintf(stderr, "%s", log);
	free(log);
}


static GLuint zglez_create_shader_from_string(const char* source, GLenum type ) {

	GLuint res = glCreateShader(type);
	const char* sources[] = { source };
	glShaderSource(res, 1, sources, NULL);
	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		fprintf(stderr, "Error compiling shader of type %d!\n", type);
		zglez_print_log(res);
		glDeleteShader(res);
		return 0;
	}

	return res;

}


static GLuint zglez_create_shader_program(const char* vs, const char* fs) {

	GLuint vsid = zglez_create_shader_from_string(vs, GL_VERTEX_SHADER);
	GLuint fsid = zglez_create_shader_from_string(fs, GL_FRAGMENT_SHADER);
	GB_ASSERT(vsid != 0 && fsid != 0);

	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vsid);
	glAttachShader(program_id, fsid);
	glLinkProgram(program_id);
	ZGLEZ_CHECK_ERROR();
	glDeleteShader(vsid);
	glDeleteShader(fsid);

	GLint status = GL_FALSE;
	glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	GB_ASSERT(status != GL_FALSE);
	
	return program_id;
}


static void zgles_create_points() {

	g_zglez_points = ZEN_CALLOC(zglez_points, 1);
	GB_ASSERT_NOT_NULL(g_zglez_points);
	zglez_points *points = g_zglez_points;

	const char* vs = \
		"#version 440\n"
		"uniform mat4 projectionMatrix;\n"
		"layout(location = 0) in vec3 v_position;\n"
		"layout(location = 1) in vec4 v_color;\n"
		"layout(location = 2) in float v_size;\n"
		"out vec4 f_color;\n"
		"void main(void)\n"
		"{\n"
		"	f_color = v_color;\n"
		"	gl_Position = projectionMatrix * vec4(v_position, 1.0f);\n"
		"   gl_PointSize = v_size;\n"
		"}\n";

	const char* fs = \
		"#version 440\n"
		"in vec4 f_color;\n"
		"out vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"	color = f_color;\n"
		"}\n";

	points->projection = Matrix4x4();

	points->program_id = zglez_create_shader_program(vs, fs);
	glUseProgram(points->program_id);
	points->projection_uniform = glGetUniformLocation(points->program_id, "projectionMatrix");
	points->vertex_attribute = 0;
	points->color_attribute = 1;
	points->size_attribute = 2;

	// Generate
	glGenVertexArrays(1, &points->vao_id);
	glGenBuffers(3, points->vbo_ids);

	glBindVertexArray(points->vao_id);
	glEnableVertexAttribArray(points->vertex_attribute);
	glEnableVertexAttribArray(points->color_attribute);
	glEnableVertexAttribArray(points->size_attribute);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, points->vbo_ids[0]);
	glVertexAttribPointer(points->vertex_attribute, 3, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(points->vertices), points->vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, points->vbo_ids[1]);
	glVertexAttribPointer(points->color_attribute, 4, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(points->colors), points->colors, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, points->vbo_ids[2]);
	glVertexAttribPointer(points->size_attribute, 1, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(points->sizes), points->sizes, GL_DYNAMIC_DRAW);

	ZGLEZ_CHECK_ERROR();

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	points->count = 0;
	
}	


static void zglez_flush_points() {

	zglez_points *points = g_zglez_points;
	if (points->count == 0)
		return;

	glUseProgram(points->program_id);

	glUniformMatrix4fv(points->projection_uniform, 1, GL_FALSE, points->projection.m);

	glBindVertexArray(points->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, points->vbo_ids[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, points->count * sizeof(Vector3_t), points->vertices);

	glBindBuffer(GL_ARRAY_BUFFER, points->vbo_ids[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, points->count * sizeof(Colorf_t), points->colors);

	glBindBuffer(GL_ARRAY_BUFFER, points->vbo_ids[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, points->count * sizeof(float), points->sizes);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, points->count);
	glDisable(GL_PROGRAM_POINT_SIZE);

	ZGLEZ_CHECK_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	points->count = 0;
}


static void zglez_point_vertex(const Vector3_t v, const Colorf_t c, float size) {

	zglez_points *points = g_zglez_points;
	if (points->count == points->max_vertices)
		zglez_flush_points();

	points->vertices[points->count] = v;
	points->colors[points->count] = c;
	points->sizes[points->count] = size;
	++points->count;

}


static void zglez_destroy_points() {

	zglez_points *points = g_zglez_points;
	if (points == NULL)
		return;

	if (points->vao_id) {
		glDeleteVertexArrays(1, &points->vao_id);
		glDeleteBuffers(3, points->vbo_ids);
	}

	if (points->program_id) {
		glDeleteProgram(points->program_id);
	}

	free(points);

}


static void zglez_create_lines() {

	g_zglez_lines = ZEN_CALLOC(zglez_lines, 1); 
	GB_ASSERT_NOT_NULL(g_zglez_lines);
	zglez_lines *lines = g_zglez_lines;

	const char* vs = \
		"#version 440\n"
		"uniform mat4 projectionMatrix;\n"
		"layout(location = 0) in vec3 v_position;\n"
		"layout(location = 1) in vec4 v_color;\n"
		"out vec4 f_color;\n"
		"void main(void)\n"
		"{\n"
		"	f_color = v_color;\n"
		"	gl_Position = projectionMatrix * vec4(v_position, 1.0f);\n"
		"}\n";

	const char* fs = \
		"#version 440\n"
		"in vec4 f_color;\n"
		"out vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"	color = f_color;\n"
		"}\n";

	lines->projection = Matrix4x4();

	lines->program_id = zglez_create_shader_program(vs, fs);
	glUseProgram(lines->program_id);
	lines->projection_uniform = glGetUniformLocation(lines->program_id, "projectionMatrix");
	lines->vertex_attribute = 0;
	lines->color_attribute = 1;

	// Generate
	glGenVertexArrays(1, &lines->vao_id);
	glGenBuffers(2, lines->vbo_ids);

	glBindVertexArray(lines->vao_id);
	glEnableVertexAttribArray(lines->vertex_attribute);
	glEnableVertexAttribArray(lines->color_attribute);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, lines->vbo_ids[0]);
	glVertexAttribPointer(lines->vertex_attribute, 3, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines->vertices), lines->vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, lines->vbo_ids[1]);
	glVertexAttribPointer(lines->color_attribute, 4, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines->colors), lines->colors, GL_DYNAMIC_DRAW);

	ZGLEZ_CHECK_ERROR();

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	lines->count = 0;
	
}	


static void zglez_flush_lines() {

	zglez_lines *lines = g_zglez_lines;
	if (lines->count == 0)
		return;

	glUseProgram(lines->program_id);

	glUniformMatrix4fv(lines->projection_uniform, 1, GL_FALSE, lines->projection.m);

	glBindVertexArray(lines->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, lines->vbo_ids[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines->count * sizeof(Vector3_t), lines->vertices);

	glBindBuffer(GL_ARRAY_BUFFER, lines->vbo_ids[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines->count * sizeof(Colorf_t), lines->colors);

	glDrawArrays(GL_LINES, 0, lines->count);

	ZGLEZ_CHECK_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	lines->count = 0;

}


static void zglez_line_vertex(const Vector3_t v, const Colorf_t c) {

	zglez_lines *lines = g_zglez_lines;
	if (lines->count >= lines->max_vertices)
		zglez_flush_lines();

	lines->vertices[lines->count] = v;
	lines->colors[lines->count] = c;
	++lines->count;

}


static void zglez_destroy_lines() {

	zglez_lines *lines = g_zglez_lines;
	if (lines == NULL)
		return;

	if (lines->vao_id) {
		glDeleteVertexArrays(1, &lines->vao_id);
		glDeleteBuffers(2, lines->vbo_ids);
	}

	if (lines->program_id) {
		glDeleteProgram(lines->program_id);
	}

	free(lines);

}

static void zglez_create_triangles() {

	g_zglez_triangles = ZEN_CALLOC(zglez_triangles, 1);
	GB_ASSERT_NOT_NULL(g_zglez_triangles);
	zglez_triangles *triangles = g_zglez_triangles;

	const char* vs = \
		"#version 440\n"
		"uniform mat4 projectionMatrix;\n"
		"layout(location = 0) in vec3 v_position;\n"
		"layout(location = 1) in vec4 v_color;\n"
		"out vec4 f_color;\n"
		"void main(void)\n"
		"{\n"
		"	f_color = v_color;\n"
		"	gl_Position = projectionMatrix * vec4(v_position, 1.0f);\n"
		"}\n";

	const char* fs = \
		"#version 440\n"
		"in vec4 f_color;\n"
		"out vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"	color = f_color;\n"
		"}\n";

	triangles->projection = Matrix4x4();

	triangles->program_id = zglez_create_shader_program(vs, fs);
	glUseProgram(triangles->program_id);
	triangles->projection_uniform = glGetUniformLocation(triangles->program_id, "projectionMatrix");
	triangles->vertex_attribute = 0;
	triangles->color_attribute = 1;

	// Generate
	glGenVertexArrays(1, &triangles->vao_id);
	glGenBuffers(2, triangles->vbo_ids);

	glBindVertexArray(triangles->vao_id);
	glEnableVertexAttribArray(triangles->vertex_attribute);
	glEnableVertexAttribArray(triangles->color_attribute);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, triangles->vbo_ids[0]);
	glVertexAttribPointer(triangles->vertex_attribute, 3, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles->vertices), triangles->vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, triangles->vbo_ids[1]);
	glVertexAttribPointer(triangles->color_attribute, 4, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles->colors), triangles->colors, GL_DYNAMIC_DRAW);

	ZGLEZ_CHECK_ERROR();

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	triangles->count = 0;
	
}	


static void zglez_flush_triangles() {

	zglez_triangles *triangles = g_zglez_triangles;
	if (triangles->count == 0)
		return;

	glUseProgram(triangles->program_id);

	glUniformMatrix4fv(triangles->projection_uniform, 1, GL_FALSE, triangles->projection.m);

	glBindVertexArray(triangles->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, triangles->vbo_ids[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, triangles->count * sizeof(Vector3_t), triangles->vertices);

	glBindBuffer(GL_ARRAY_BUFFER, triangles->vbo_ids[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, triangles->count * sizeof(Colorf_t), triangles->colors);

	glDrawArrays(GL_TRIANGLES, 0, triangles->count);

	ZGLEZ_CHECK_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	triangles->count = 0;

}


static void zglez_triangle_vertex(const Vector3_t v, const Colorf_t c) {

	zglez_triangles *triangles = g_zglez_triangles;
	if (triangles->count >= triangles->max_vertices)
		zglez_flush_triangles();

	triangles->vertices[triangles->count] = v;
	triangles->colors[triangles->count] = c;
	++triangles->count;

}


static void zglez_destroy_triangles() {

	zglez_triangles *triangles = g_zglez_triangles;
	if (triangles == NULL)
		return;

	if (triangles->vao_id) {
		glDeleteVertexArrays(1, &triangles->vao_id);
		glDeleteBuffers(2, triangles->vbo_ids);
	}

	if (triangles->program_id) {
		glDeleteProgram(triangles->program_id);
	}

	free(triangles);

}


ZGLEZDEF void zglez_create_textures() {

	g_zglez_textures = ZEN_CALLOC(zglez_textures, 1);
	GB_ASSERT_NOT_NULL(g_zglez_textures);
	zglez_textures *tex = g_zglez_textures;

	tex->map = zglez_texmap_create();

	const char *vs = \
		"#version 440 core\n"
		"uniform mat4 projectionMatrix;\n"
		"layout (location = 0) in vec3 v_position;\n"
		"layout (location = 1) in vec2 v_tex_coord;\n"
		"out vec2 f_tex_coord;\n"
		"void main(void) {\n"
		"	gl_Position = projectionMatrix * vec4(v_position, 1.0f);\n"
		"	f_tex_coord = v_tex_coord;\n"
		"}\n";

	const char *fs = \
		"#version 440 core\n"
		"in vec2 f_tex_coord;\n"
		"layout (binding = 0) uniform sampler2D u_tex;\n"
		"out vec4 o_colour;\n"
		"void main(void) {\n"
		"	o_colour = texture2D(u_tex, f_tex_coord);\n"
		"}\n";

	tex->projection = Matrix4x4();

	tex->program_id = zglez_create_shader_program(vs, fs);
	glUseProgram(tex->program_id);
	tex->projection_uniform = glGetUniformLocation(tex->program_id, "projectionMatrix");
	tex->sampler_location = glGetUniformLocation(tex->program_id, "u_tex");
	tex->vertex_attribute = 0;
	tex->tex_attribute = 1;

	// Generate
	glGenVertexArrays(1, &tex->vao_id);
	glGenBuffers(2, tex->vbo_ids);

	glBindVertexArray(tex->vao_id);
	glEnableVertexAttribArray(tex->vertex_attribute);
	glEnableVertexAttribArray(tex->tex_attribute);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, tex->vbo_ids[0]);
	glVertexAttribPointer(tex->vertex_attribute, 3, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex->vertices), tex->vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, tex->vbo_ids[1]);
	glVertexAttribPointer(tex->tex_attribute, 2, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex->tex_coords), tex->tex_coords, GL_DYNAMIC_DRAW);

	ZGLEZ_CHECK_ERROR();

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	tex->count = 0;

}


ZGLEZDEF void zglez_flush_textures() {

	zglez_textures *textures = g_zglez_textures;
	if (textures->count == 0 || textures->current_texture == NULL)
		return;


	glUseProgram(textures->program_id);
	glBindVertexArray(textures->vao_id);

	zglez_texture_t *texture = textures->current_texture;
	glUniformMatrix4fv(textures->projection_uniform, 1, GL_FALSE, textures->projection.m);
	glUniform1i(textures->sampler_location, texture->active_texture);

	glActiveTexture(GL_TEXTURE0 + texture->active_texture);
	glBindSampler(texture->active_texture, texture->sampler);

	glBindTexture(GL_TEXTURE_2D, texture->handle);

	glBindBuffer(GL_ARRAY_BUFFER, textures->vbo_ids[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, textures->count * sizeof(Vector3_t), textures->vertices);

	glBindBuffer(GL_ARRAY_BUFFER, textures->vbo_ids[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, textures->count * sizeof(Vector2_t), textures->tex_coords);

	glDrawArrays(GL_TRIANGLES, 0, textures->count);

	ZGLEZ_CHECK_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindSampler(0, 0);
	glUseProgram(0);

	textures->count = 0;

}


static void zglez_texture_vertex(zglez_texture_t *texture, Vector3_t v, Vector2_t t) {

	zglez_textures *textures = g_zglez_textures;
	if (textures->count >= textures->max_vertices || textures->current_texture != texture)
		zglez_flush_textures();

	textures->current_texture = texture;
	textures->vertices[textures->count] = v;
	textures->tex_coords[textures->count] = t;

	textures->count++;

}


static void zglez_delete_texture(zglez_texture_t *texture) {

	glDeleteTextures(1, &texture->handle);
	glDeleteSamplers(1, &texture->sampler);

}


static void zglez_destroy_textures() {

	zglez_textures *textures = g_zglez_textures;
	if (textures == NULL)
		return;

	const char ** keys = NULL;
	zglez_texture_map *map = g_zglez_textures->map;
	for (int i = 0; i < map->limit; ++i) {
		if (map->table[i].k)
			stb_arr_push(keys, map->table[i].k);	
	}

	zglez_texture_t *texture;
	for (int i = 0; i < stb_arr_len(keys); ++i) {
		if (zglez_texmap_remove(map, keys[i], &texture)) {
			zglez_delete_texture(texture);
			free(texture);
		}
	}

	zglez_texmap_destroy(textures->map);


	if (textures->vao_id) {
		glDeleteVertexArrays(1, &textures->vao_id);
		glDeleteBuffers(2, textures->vbo_ids);
	}

	if (textures->program_id) {
		glDeleteProgram(textures->program_id);
	}

	free(textures);
	stb_arr_free(keys);

}


ZGLEZDEF int zglez_load_texture_from_memory(const char *name, void const *memory, int32 width, int32 height, int32 channel_count) {


	zglez_texture_map *map = g_zglez_textures->map;
	zglez_texture_t *old_texture = zglez_texmap_get(map, name);
	if (old_texture != NULL) {
		zglez_unload_texture(name);
	}

	zglez_texture_t *texture = ZEN_CALLOC(zglez_texture_t, 1);
	GB_ASSERT_NOT_NULL(texture);
	zglez_texmap_set(map, name, texture);

	texture->width = width;
	texture->height = height;
	texture->channel_count = channel_count;
	texture->active_texture = 0;

	glGenTextures(1, &texture->handle);
	glBindTexture(GL_TEXTURE_2D, texture->handle);


	glGenSamplers(1, &texture->sampler);
	glSamplerParameteri(texture->sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(texture->sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(texture->sampler, GL_TEXTURE_WRAP_S,     GL_REPEAT);
	glSamplerParameteri(texture->sampler, GL_TEXTURE_WRAP_T,     GL_REPEAT);


	GB_ASSERT(GL_MAX_TEXTURE_SIZE > width);
	GB_ASSERT(GL_MAX_TEXTURE_SIZE > height);

	glTexImage2D(
		GL_TEXTURE_2D, 0, zglez_internal_texture_format[channel_count-1], 
		width, height, 0, zglez_texture_format[channel_count-1], 
		GL_UNSIGNED_BYTE, memory);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFinish();

	return 1;

}

ZGLEZDEF int zglez_load_texture_from_file(const char *name, const char * path_to_file, int *w, int *h, int flip_vertically) {

	int width, height, comp;
	stbi_set_flip_vertically_on_load(flip_vertically);
	uint8 *data = stbi_load(path_to_file, &width, &height, &comp, 4);
	if (data) {
		zglez_load_texture_from_memory(name, data, width, height, 4);
		stbi_image_free(data);
		if (w) *w = width;
		if (h) *h = height;
		return 1;
	}

	return 0;

}


ZGLEZDEF int zglez_unload_texture(const char *name) {
	zglez_texture_t *texture = NULL;
	int result = zglez_texmap_remove(g_zglez_textures->map, name, &texture);
	if (result) {
		zglez_delete_texture(texture);
		free(texture);
	}
	return result;
}


ZGLEZDEF void zglez_unload_all_textures() {

	const char ** keys = NULL;
	zglez_texture_map *map = g_zglez_textures->map;
	for (int i = 0; i < map->limit; ++i) {
		if (map->table[i].k)
			stb_arr_push(keys, map->table[i].k);	
	}

	zglez_texture_t *texture = NULL;
	for (int i = 0; i < stb_arr_len(keys); ++i) {
		if (zglez_texmap_remove(map, keys[i], &texture)) {
			zglez_delete_texture(texture);
			free(texture);
			texture = NULL;
		}
	}

}


ZGLEZDEF void zglez_create_fonts() {

	g_zglez_fonts = ZEN_CALLOC(zglez_fonts, 1);	
	GB_ASSERT_NOT_NULL(g_zglez_fonts);
	zglez_fonts *fonts = g_zglez_fonts;

	fonts->map = zglez_fontmap_create();

	const char *vs = \
		"#version 440 core\n"
		"uniform mat4 proj_mat;\n"
		"layout (location = 0) in vec3 v_position;\n"
		"layout (location = 1) in vec2 v_tex_coord;\n"
		"layout (location = 2) in vec3 v_color;\n"
		"out vec2 f_tex_coord;\n"
		"out vec3 f_color;\n"
		"void main(void) {\n"
		"	gl_Position = proj_mat * vec4(v_position, 1.0f);\n"
		"	f_tex_coord = v_tex_coord;\n"
		"	f_color = v_color;\n"
		"}\n";

	const char *fs = \
		"#version 440 core\n"
		"in vec2 f_tex_coord;\n"
		"in vec3 f_color;\n"
		"layout (binding = 0) uniform sampler2D u_tex;\n"
		"out vec4 o_colour;\n"
		"void main(void) {\n"
		"  float alpha = texture2D(u_tex, f_tex_coord).r;"
		"	o_colour = vec4(f_color * alpha, alpha);\n"
		"}\n";

	fonts->projection = Matrix4x4();

	fonts->program_id = zglez_create_shader_program(vs, fs);
	glUseProgram(fonts->program_id);
	fonts->projection_uniform = glGetUniformLocation(fonts->program_id, "proj_mat");
	fonts->sampler_location = glGetUniformLocation(fonts->program_id, "u_tex");
	fonts->vertex_attribute = 0;
	fonts->tex_attribute = 1;
	fonts->color_attribute = 2;

	// Generate
	glGenVertexArrays(1, &fonts->vao_id);
	glGenBuffers(3, fonts->vbo_ids);

	glBindVertexArray(fonts->vao_id);
	glEnableVertexAttribArray(fonts->vertex_attribute);
	glEnableVertexAttribArray(fonts->tex_attribute);
	glEnableVertexAttribArray(fonts->color_attribute);


	glBindBuffer(GL_ARRAY_BUFFER, fonts->vbo_ids[0]);
	glVertexAttribPointer(fonts->vertex_attribute, 3, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(fonts->vertices), fonts->vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, fonts->vbo_ids[1]);
	glVertexAttribPointer(fonts->tex_attribute, 2, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(fonts->tex_coords), fonts->tex_coords, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, fonts->vbo_ids[2]);
	glVertexAttribPointer(fonts->color_attribute, 4, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(fonts->colors), fonts->colors, GL_DYNAMIC_DRAW);

	ZGLEZ_CHECK_ERROR();

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	fonts->count = 0;

}


ZGLEZDEF void zglez_flush_fonts() {

	zglez_fonts *fonts = g_zglez_fonts;
	if (fonts->count == 0 || fonts->current_font == NULL)
		return;


	glUseProgram(fonts->program_id);
	glBindVertexArray(fonts->vao_id);

	zglez_font_t *font = fonts->current_font;
	glUniformMatrix4fv(fonts->projection_uniform, 1, GL_FALSE, fonts->projection.m);
	glUniform1i(fonts->sampler_location, font->active_texture);

	glActiveTexture(GL_TEXTURE0 + font->active_texture);
	glBindSampler(font->active_texture, font->sampler);

	glBindTexture(GL_TEXTURE_2D, font->handle);

	glBindBuffer(GL_ARRAY_BUFFER, fonts->vbo_ids[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, fonts->count * sizeof(Vector3_t), fonts->vertices);

	glBindBuffer(GL_ARRAY_BUFFER, fonts->vbo_ids[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, fonts->count * sizeof(Vector2_t), fonts->tex_coords);

	glBindBuffer(GL_ARRAY_BUFFER, fonts->vbo_ids[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, fonts->count * sizeof(Colorf_t), fonts->colors);

	glDrawArrays(GL_TRIANGLES, 0, fonts->count);

	ZGLEZ_CHECK_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindSampler(0, 0);
	glUseProgram(0);

	fonts->count = 0;

}



static void zglez_delete_font_texture(zglez_font_t *font) {

	glDeleteTextures(1, &font->handle);
	glDeleteSamplers(1, &font->sampler);

}


static void zglez_destroy_fonts() {

	zglez_fonts *fonts = g_zglez_fonts;
	if (fonts == NULL)
		return;

	const char ** keys = NULL;
	zglez_font_map *map = fonts->map;
	for (int i = 0; i < map->limit; ++i) {
		if (map->table[i].k)
			stb_arr_push(keys, map->table[i].k);	
	}

	zglez_font_t *font;
	for (int i = 0; i < stb_arr_len(keys); ++i) {
		if (zglez_fontmap_remove(map, keys[i], &font)) {
			zglez_delete_font_texture(font);
			free(font);
		}
	}

	zglez_fontmap_destroy(fonts->map);


	if (fonts->vao_id) {
		glDeleteVertexArrays(1, &fonts->vao_id);
		glDeleteBuffers(3, fonts->vbo_ids);
	}

	if (fonts->program_id) {
		glDeleteProgram(fonts->program_id);
	}

	free(fonts);
	stb_arr_free(keys);

}


ZGLEZDEF int zglez_load_font_from_memory(const char *name, unsigned char *memory, isize size_in_bytes, 
		int pixel_size, int padding, int oversample) {

	zglez_font_map *map = g_zglez_fonts->map;
	zglez_font_t *old_font = zglez_fontmap_get(map, name);
	if (old_font != NULL) {
		zglez_unload_font(name);
	}


	zglez_font_t *font = ZEN_CALLOC(zglez_font_t, 1);
	GB_ASSERT_NOT_NULL(font);
	zglez_fontmap_set(map, name, font);


	// TODO:Need a better way to figure out texture size
	font->width = ZGLEZ_FONT_WIDTH;
	font->height = ZGLEZ_FONT_HEIGHT;
	font->active_texture = 0;

	GB_ASSERT(stbtt_InitFont(&font->font, memory, stbtt_GetFontOffsetForIndex(memory,0)));

	int ascii_start = ' ';
	int ascii_end = '~';
	int ascii_count = ascii_end - ascii_start + 1;
	ziout(ascii_count);
	int ext_start = 0xA0;
	int ext_end = 0xFF;
	int ext_count = ext_end - ext_start + 1;
	ziout(ext_count);

	// pdata = [ascii_count + ext_count]
	font->pdata = ZEN_CALLOC(stbtt_packedchar, ascii_count + ext_count);
	uint8 *texture_data = ZEN_CALLOC(uint8, font->width * font->height);
	GB_ASSERT_NOT_NULL(texture_data);

	stbtt_pack_context pc;
	stbtt_PackBegin(&pc, texture_data, font->width, font->height, 0, padding, NULL);
	stbtt_PackSetOversampling(&pc, oversample, oversample);
	stbtt_PackFontRange(&pc, memory, 0, pixel_size, ascii_start, ascii_count, font->pdata);
	stbtt_PackFontRange(&pc, memory, 0, pixel_size, ext_start, ext_count, font->pdata + ascii_count);
	stbtt_PackEnd(&pc);

	free(memory);

	glGenTextures(1, &font->handle);
	glBindTexture(GL_TEXTURE_2D, font->handle);


	glGenSamplers(1, &font->sampler);
	glSamplerParameteri(font->sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(font->sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(font->sampler, GL_TEXTURE_WRAP_S,     GL_REPEAT);
	glSamplerParameteri(font->sampler, GL_TEXTURE_WRAP_T,     GL_REPEAT);


	GB_ASSERT(GL_MAX_TEXTURE_SIZE > font->width);
	GB_ASSERT(GL_MAX_TEXTURE_SIZE > font->height);

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RED, font->width, font->height, 
		0, GL_RED, GL_UNSIGNED_BYTE, texture_data
	);
	free(texture_data);

	glBindTexture(GL_TEXTURE_2D, 0);
	glFinish();

	return 1;


}


ZGLEZDEF int zglez_unload_font(const char *name) {
	zglez_font_t *font = NULL;
	int result = zglez_fontmap_remove(g_zglez_fonts->map, name, &font);
	if (result) {
		zglez_delete_font_texture(font);
		free(font);
	}
	return result;
}


ZGLEZDEF void zglez_unload_all_fonts() {

	const char ** keys = NULL;
	zglez_font_map *map = g_zglez_fonts->map;
	for (int i = 0; i < map->limit; ++i) {
		if (map->table[i].k)
			stb_arr_push(keys, map->table[i].k);	
	}

	zglez_font_t *font = NULL;
	for (int i = 0; i < stb_arr_len(keys); ++i) {
		if (zglez_fontmap_remove(map, keys[i], &font)) {
			zglez_delete_font_texture(font);
			free(font);
			font = NULL;
		}
	}

}


ZGLEZDEF void zglez_draw_point(Vector3_t v, Colorf_t c, float pt_size) {
	zglez_point_vertex(v, c, pt_size);
}


ZGLEZDEF void zglez_draw_line(Vector3_t v0, Vector3_t v1, Colorf_t c) {
	zglez_line_vertex(v0, c);
	zglez_line_vertex(v1, c);
}


ZGLEZDEF void zglez_draw_polygon(Vector3_t *v, Colorf_t c, int count) {

	Vector3_t V0 = v[count - 1];
	for (int i = 0; i < count; ++i) {
		Vector3_t V1 = v[i];
		zglez_draw_line(V0, V1, c);
		V0 = V1;
	}

}


void zglez_fill_polygon(Vector3_t *v, Colorf_t c, int count, bool blend) {

	Colorf_t color = blend ? c * 0.5f : c;
	Vector3_t VM = v[0];
	Vector3_t VA = v[count - 1];
	for (int i = 0; i < count - 1; ++i) {
		Vector3_t VB = v[i+1];
		zglez_triangle_vertex(VM, color);
		zglez_triangle_vertex(VA, color);
		zglez_triangle_vertex(VB, color);
		VA = VB;
	}

	Vector3_t V1 = v[count - 1];
	for (int i = 1; i < count; ++i) {
		Vector3_t V0 = v[i];
		zglez_line_vertex(V0, c);
		zglez_line_vertex(V1, c);
		V1 = V0;
	}

}



ZGLEZDEF void zglez_draw_circle(Vector3_t center, float radius, Colorf_t c, int steps) {

	float delta = 2.0f * M_PI / cast(float)steps;
	float x = radius * cosf(0 * delta);
	float y = radius * sinf(0 * delta);
	Vector3_t V0 = Vector3(x, y, center.z);
	for (int i = 1; i <= steps; ++i) {
		x = radius * cosf(i * delta);
		y = radius * sinf(i * delta);
		Vector3_t V1 = Vector3(x, y, center.z);
		zglez_draw_line(V0, V1, c);
		V0 = V1;
	}

}


void zglez_fill_circle(Vector3_t center, float radius, Colorf_t c, int steps, bool blend) {

	float delta = 2.0f * M_PI / cast(float)steps;

	float x = radius * cosf(0 * delta);
	float y = radius * sinf(0 * delta);
	Vector3_t V0 = Vector3(x, y, center.z);
	Colorf_t color = blend ? c * 0.5f : c;
	for (int i = 1; i <= steps; ++i) {
		x = radius * cosf(i * delta);
		y = radius * sinf(i * delta);
		Vector3_t V1 = Vector3(x, y, center.z);
		zglez_triangle_vertex(center, color);
		zglez_triangle_vertex(V0, color);
		zglez_triangle_vertex(V1, color);
		V0 = V1;
	}


	x = radius * cosf(0 * delta);
	y = radius * sinf(0 * delta);
	V0 = Vector3(x, y, center.z);
	for (int i = 1; i <= steps; ++i) {
		x = radius * cosf(i * delta);
		y = radius * sinf(i * delta);
		Vector3_t V1 = Vector3(x, y, center.z);
		zglez_draw_line(V0, V1, c);
		V0 = V1;
	}

}


ZGLEZDEF void zglez_texture_quad(const char *name, Vector3_t *v, Vector2_t *t) {

	zglez_texture_map *map = g_zglez_textures->map;
	zglez_texture_t *texture = zglez_texmap_get(map, name);
	GB_ASSERT_MSG(texture != NULL, "Texture %s not found!", name);

	zglez_texture_vertex(texture, v[0], t[0]);
	zglez_texture_vertex(texture, v[1], t[1]);
	zglez_texture_vertex(texture, v[3], t[3]);

	zglez_texture_vertex(texture, v[3], t[3]);
	zglez_texture_vertex(texture, v[1], t[1]);
	zglez_texture_vertex(texture, v[2], t[2]);

}


static void zglez_font_vertex(zglez_font_t *font, float x, float y, float s, float t, Colorf_t c) {

	zglez_fonts *fonts = g_zglez_fonts;
	if (fonts->count >= fonts->max_vertices || fonts->current_font  != font)
		zglez_flush_fonts();

	fonts->current_font = font;
	fonts->vertices[fonts->count] = Vector3(x, y, 0);
	fonts->tex_coords[fonts->count] = Vector2(s, t);
	fonts->colors[fonts->count] = c;

	fonts->count++;

}


static int zglez_get_char_index(int ch) {

	// @TODO: is there a better way?
	int ascii_start = ' ';
	int ascii_end = '~';
	int ascii_count = ascii_end - ascii_start + 1;
	int ext_start = 0xA0;
	int ext_end = 0xFF;

	if (ch >= ascii_start && ch <= ascii_end) {
		return ch - ascii_start;
	} else if(ch >= ext_start && ch <= ext_end) {
		return ch - ext_start + ascii_count;	
	} else {
		return -1;
	}

}


ZGLEZDEF void zglez_draw_text(const char *name, const char *string, Vector2_t position, Colorf_t color) {

	zglez_fonts * fonts = g_zglez_fonts;
	zglez_font_t *font = zglez_fontmap_get(fonts->map, name);
	GB_ASSERT_NOT_NULL(font);

	int ch = 0;
	stbtt_aligned_quad q;
	float xpos = position.x;
	float ypos = position.y;

	while (string[ch]) {
		int char_index = zglez_get_char_index(string[ch]);
		stbtt_GetPackedQuad(font->pdata, font->width, font->height, char_index, &xpos, &ypos, &q, 1);

		zglez_font_vertex(font, q.x0, q.y0, q.s0, q.t0, color);
		zglez_font_vertex(font, q.x0, q.y1, q.s0, q.t1, color);
		zglez_font_vertex(font, q.x1, q.y0, q.s1, q.t0, color);

		zglez_font_vertex(font, q.x1, q.y0, q.s1, q.t0, color);
		zglez_font_vertex(font, q.x0, q.y1, q.s0, q.t1, color);
		zglez_font_vertex(font, q.x1, q.y1, q.s1, q.t1, color);
		ch++;

	}

}


ZGLEZDEF void zglez_init() {

	zgles_create_points();
	zglez_create_lines();
	zglez_create_triangles();
	zglez_create_textures();
	zglez_create_fonts();

}


ZGLEZDEF void zglez_projection(Matrix4x4_t mat) {

	g_zglez_lines->projection = mat;
	g_zglez_points->projection = mat;
	g_zglez_triangles->projection = mat;
	g_zglez_textures->projection = mat;

}


ZGLEZDEF void zglez_text_projection(Matrix4x4_t mat) {
	g_zglez_fonts->projection = mat;
}


ZGLEZDEF void zglez_flush() {

	zglez_flush_textures();
	zglez_flush_triangles();
	zglez_flush_lines();
	zglez_flush_points();
	zglez_flush_fonts();

}


ZGLEZDEF void zglez_quit() {

	zglez_destroy_points();
	zglez_destroy_lines();
	zglez_destroy_triangles();
	zglez_destroy_textures();
	zglez_destroy_fonts();

}


#endif
