#if !defined(__ZEN_GLEZ_H__)
#define __ZEN_GLEZ_H__


#if defined(ZEN_LIB_DEV)
#include "glad/glad.h"
#include "zen_lib/zen.h"
#include "zen_lib/zen_math.h"
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
ZGLEZDEF void zglez_projection(float m[16]);

ZGLEZDEF void zglez_point(Vector3_t v, Colorf_t c, float pt_size);
ZGLEZDEF void zglez_line(Vector3_t v0, Colorf_t c0, Vector3_t v1, Colorf_t c1);

// TODO
// polygons
// filled polygons
// circles
// filled circles
// textures


#if defined(__cplusplus)
}
#endif


#endif // __ZEN_GLEZ_H__


#if defined(ZEN_GLEZ_IMPLEMENTATION)


#define ZGLEZ_CHECK_ERROR() do { \
	GLenum error_code = glGetError(); \
	if (error_code != GL_NO_ERROR) { \
		GB_PANIC("OpenGL error = %d\n", error_code); \
	} \
} while (0)


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


static void zgles_create_points() {

	g_zglez_points = (zglez_points *)calloc(1, zen_sizeof(zglez_points));
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

	g_zglez_lines = (zglez_lines *)calloc(1, zen_sizeof(zglez_lines));
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


ZGLEZDEF void zglez_point(Vector3_t v, Colorf_t c, float pt_size) {
	zglez_point_vertex(v, c, pt_size);
}


ZGLEZDEF void zglez_line(Vector3_t v0, Colorf_t c0, Vector3_t v1, Colorf_t c1) {
	zglez_line_vertex(v0, c0);
	zglez_line_vertex(v1, c1);
}


ZGLEZDEF void zglez_init() {

	zgles_create_points();
	zglez_create_lines();

}

ZGLEZDEF void zglez_projection(Matrix4x4_t mat) {

	g_zglez_lines->projection = mat;
	g_zglez_points->projection = mat;

}

ZGLEZDEF void zglez_flush() {

	zglez_flush_points();
	zglez_flush_lines();

}


ZGLEZDEF void zglez_quit() {

	zglez_destroy_points();
	zglez_destroy_lines();

}


//void zgl_render_polygon(Vector3_t *v, int count, Colorf_t c) {
	//Vector3_t *S = v[count - 1];
	//for (int i = 0; i < count; ++i) {
		//Vector3_t *P = v[i];
		//zgl_render_line(S, P, c);
		//S = P;
	//}
//}

#endif
