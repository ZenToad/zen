#if !defined(__ZEN_GLEZ_H__)
#define __ZEN_GLEZ_H__


#if defined(ZEN_LIB_DEV)
#include "glad/glad.h"
#include "zen.h"
#include "zen_math.h"
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

ZGLEZDEF void zglez_render_point(Vector2_t v, Colorf_t c, float pt_size);
ZGLEZDEF void zglez_render_line(Vector2_t v0, Colorf_t c0, Vector2_t v1, Colorf_t c1);


#if defined(__cplusplus)
}
#endif


#endif // __ZEN_GLEZ_H__


#if defined(ZEN_GLEZ_IMPLEMENTATION)


#define SAFEGL_CHECK_ERROR() do { \
	GLenum error_code = glGetError(); \
	if (error_code != GL_NO_ERROR) { \
		GB_PANIC("OpenGL error = %d\n", error_code); \
	} \
} while (0)


typedef struct zglez_render_points {
 
	enum { max_vertices = 512 };
	Vector2_t vertices[max_vertices];
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

} zglez_render_points;
zglez_render_points *__zenglez_rpts;


typedef struct zglez_render_lines {
 
	enum { max_vertices = 512 * 2 };
	Vector2_t vertices[max_vertices];
	Colorf_t colors[max_vertices];
 
	Matrix4x4_t projection;

	int32 count;
    
	GLuint vao_id;
	GLuint vbo_ids[2];
	GLuint program_id;
	GLint projection_uniform;
	GLint vertex_attribute;
	GLint color_attribute;

} zglez_render_lines;
zglez_render_lines *__zenglez_rlines;


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

	glDeleteShader(vsid);
	glDeleteShader(fsid);

	GLint status = GL_FALSE;
	glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	GB_ASSERT(status != GL_FALSE);
	
	return program_id;
}


static void zglez_create_render_points() {

	__zenglez_rpts = (zglez_render_points *)calloc(1, zen_sizeof(zglez_render_points));
	GB_ASSERT_NOT_NULL(__zenglez_rpts);
	zglez_render_points *rpts = __zenglez_rpts;

	const char* vs = \
		"#version 440\n"
		"uniform mat4 projectionMatrix;\n"
		"layout(location = 0) in vec2 v_position;\n"
		"layout(location = 1) in vec4 v_color;\n"
		"layout(location = 2) in float v_size;\n"
		"out vec4 f_color;\n"
		"void main(void)\n"
		"{\n"
		"	f_color = v_color;\n"
		"	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
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

	rpts->projection = Matrix4x4();

	rpts->program_id = zglez_create_shader_program(vs, fs);
	rpts->projection_uniform = glGetUniformLocation(rpts->program_id, "projectionMatrix");
	rpts->vertex_attribute = 0;
	rpts->color_attribute = 1;
	rpts->size_attribute = 2;

	// Generate
	glGenVertexArrays(1, &rpts->vao_id);
	glGenBuffers(3, rpts->vbo_ids);

	glBindVertexArray(rpts->vao_id);
	glEnableVertexAttribArray(rpts->vertex_attribute);
	glEnableVertexAttribArray(rpts->color_attribute);
	glEnableVertexAttribArray(rpts->size_attribute);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, rpts->vbo_ids[0]);
	glVertexAttribPointer(rpts->vertex_attribute, 2, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(rpts->vertices), rpts->vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, rpts->vbo_ids[1]);
	glVertexAttribPointer(rpts->color_attribute, 4, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(rpts->colors), rpts->colors, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, rpts->vbo_ids[2]);
	glVertexAttribPointer(rpts->size_attribute, 1, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(rpts->sizes), rpts->sizes, GL_DYNAMIC_DRAW);

	SAFEGL_CHECK_ERROR();

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	rpts->count = 0;
	
}	


static void zglez_flush_render_points() {

	zglez_render_points *rpts = __zenglez_rpts;
	if (rpts->count == 0)
		return;

	glUseProgram(rpts->program_id);

	glUniformMatrix4fv(rpts->projection_uniform, 1, GL_FALSE, rpts->projection.m);

	glBindVertexArray(rpts->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, rpts->vbo_ids[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, rpts->count * sizeof(Vector2_t), rpts->vertices);

	glBindBuffer(GL_ARRAY_BUFFER, rpts->vbo_ids[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, rpts->count * sizeof(Color_t), rpts->colors);

	glBindBuffer(GL_ARRAY_BUFFER, rpts->vbo_ids[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, rpts->count * sizeof(float), rpts->sizes);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, rpts->count);
	glDisable(GL_PROGRAM_POINT_SIZE);

	SAFEGL_CHECK_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	rpts->count = 0;
}


static void zglez_add_render_point(zglez_render_points *rpts, const Vector2_t v, const Colorf_t c, float size) {

	if (rpts->count == rpts->max_vertices)
		zglez_flush_render_points();

	rpts->vertices[rpts->count] = v;
	rpts->colors[rpts->count] = c;
	rpts->sizes[rpts->count] = size;
	++rpts->count;

}


static void zglez_destroy_render_points() {

	zglez_render_points *rpts = __zenglez_rpts;
	if (rpts == NULL)
		return;

	if (rpts->vao_id) {
		glDeleteVertexArrays(1, &rpts->vao_id);
		glDeleteBuffers(3, rpts->vbo_ids);
	}

	if (rpts->program_id) {
		glDeleteProgram(rpts->program_id);
	}

	free(rpts);

}


static void zglez_create_render_lines() {

	__zenglez_rlines = (zglez_render_lines *)calloc(1, zen_sizeof(zglez_render_lines));
	GB_ASSERT_NOT_NULL(__zenglez_rlines);
	zglez_render_lines *rlines = __zenglez_rlines;

	const char* vs = \
		"#version 440\n"
		"uniform mat4 projectionMatrix;\n"
		"layout(location = 0) in vec2 v_position;\n"
		"layout(location = 1) in vec4 v_color;\n"
		"out vec4 f_color;\n"
		"void main(void)\n"
		"{\n"
		"	f_color = v_color;\n"
		"	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
		"}\n";

	const char* fs = \
		"#version 440\n"
		"in vec4 f_color;\n"
		"out vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"	color = f_color;\n"
		"}\n";

	rlines->projection = Matrix4x4();

	rlines->program_id = zglez_create_shader_program(vs, fs);
	rlines->projection_uniform = glGetUniformLocation(rlines->program_id, "projectionMatrix");
	rlines->vertex_attribute = 0;
	rlines->color_attribute = 1;

	// Generate
	glGenVertexArrays(1, &rlines->vao_id);
	glGenBuffers(2, rlines->vbo_ids);

	glBindVertexArray(rlines->vao_id);
	glEnableVertexAttribArray(rlines->vertex_attribute);
	glEnableVertexAttribArray(rlines->color_attribute);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, rlines->vbo_ids[0]);
	glVertexAttribPointer(rlines->vertex_attribute, 2, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(rlines->vertices), rlines->vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, rlines->vbo_ids[1]);
	glVertexAttribPointer(rlines->color_attribute, 4, GL_FLOAT, GL_FALSE, 0, zen_offset(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(rlines->colors), rlines->colors, GL_DYNAMIC_DRAW);

	SAFEGL_CHECK_ERROR();

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	rlines->count = 0;
	
}	


static void zglez_flush_render_lines() {

	zglez_render_lines *rlines = __zenglez_rlines;
	if (rlines->count == 0)
		return;

	glUseProgram(rlines->program_id);

	glUniformMatrix4fv(rlines->projection_uniform, 1, GL_FALSE, rlines->projection.m);

	glBindVertexArray(rlines->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, rlines->vbo_ids[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, rlines->count * sizeof(Vector2_t), rlines->vertices);

	glBindBuffer(GL_ARRAY_BUFFER, rlines->vbo_ids[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, rlines->count * sizeof(Color_t), rlines->colors);

	glDrawArrays(GL_LINES, 0, rlines->count);

	SAFEGL_CHECK_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	rlines->count = 0;

}


static void zglez_add_render_line(zglez_render_lines *rlines, const Vector2_t v0, const Colorf_t c0, Vector2_t v1, Colorf_t c1) {

	if (rlines->count + 2 >= rlines->max_vertices)
		zglez_flush_render_lines();

	rlines->vertices[rlines->count] = v0;
	rlines->colors[rlines->count] = c0;
	++rlines->count;

	rlines->vertices[rlines->count] = v1;
	rlines->colors[rlines->count] = c1;
	++rlines->count;

}


static void zglez_destroy_render_lines() {

	zglez_render_lines *rlines = __zenglez_rlines;
	if (rlines == NULL)
		return;

	if (rlines->vao_id) {
		glDeleteVertexArrays(1, &rlines->vao_id);
		glDeleteBuffers(2, rlines->vbo_ids);
	}

	if (rlines->program_id) {
		glDeleteProgram(rlines->program_id);
	}

	free(rlines);

}


ZGLEZDEF void zglez_render_point(Vector2_t v, Colorf_t c, float pt_size) {
	zglez_add_render_point(__zenglez_rpts, v, c, pt_size);
}


ZGLEZDEF void zglez_render_line(Vector2_t v0, Colorf_t c0, Vector2_t v1, Colorf_t c1) {
	zglez_add_render_line(__zenglez_rlines, v0, c0, v1, c1);
}


ZGLEZDEF void zglez_init() {

	zglez_create_render_points();
	zglez_create_render_lines();

}


ZGLEZDEF void zglez_flush() {

	zglez_flush_render_points();
	zglez_flush_render_lines();

}


ZGLEZDEF void zglez_quit() {

	zglez_destroy_render_points();
	zglez_destroy_render_lines();

}


//void zgl_render_line(Vector2_t *a, Vector2_t *b, Colorf c) {

//}

//void zgl_render_polygon(Vector2_t *v, int count, Colorf_t c) {
	//Vector2_t *S = v[count - 1];
	//for (int i = 0; i < count; ++i) {
		//Vector2_t *P = v[i];
		//zgl_render_line(S, P, c);
		//S = P;
	//}
//}

//void zgl_flush() {
	//zgl_flush_render_points(__zgl_render_points);
	//zgl_flush_render_lines(__zgl_render_lines);
	//zgl_flush_render_polygons(__zgl_render_polygons);

	//// textures...
//}

#endif
