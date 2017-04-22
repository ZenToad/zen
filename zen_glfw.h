#ifndef __ZEN_GLFW_H__
#define __ZEN_GLFW_H__

#ifdef ZEN_GLFW_STATIC
#define ZGLFWDEF static
#else
#define ZGLFWDEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct ZGLFW {

   const char *window_title;

   int major_version;
   int minor_version;
   int window_width;
	float window_widthf;
   int window_height;
	float window_heightf;
   int window_cursor_mode;
   int window_background;
   int should_close;

	double current_time;
	double last_time;
	double delta_time;

	float mouse_x;
	float mouse_y;
	float mouse_dx;
	float mouse_dy;
	float last_mouse_x;
	float last_mouse_y;

   GLFWwindow* window;
   void (*error_callback)(int, const char*);
   void (*key_callback)(GLFWwindow*, int, int, int, int);
   void (*window_size_callback)(GLFWwindow*, int, int);

	int keys[GLFW_KEY_LAST];
	

} ZGLFW;

ZGLFWDEF void zglfw_init(ZGLFW *glfw);
ZGLFWDEF void zglfw_show_window(ZGLFW &glfw);
ZGLFWDEF int zglfw_is_running(ZGLFW *glfw);
ZGLFWDEF void zglfw_begin(ZGLFW *glfw);
ZGLFWDEF void zglfw_end(ZGLFW *glfw);
ZGLFWDEF void zglfw_quit(ZGLFW *glfw);

#ifdef __cplusplus
}
#endif 

#endif //__ZEN_GLFW_H__

//------------------------------------------ 
// Implementation
//------------------------------------------ 
//
#ifdef ZEN_GLFW_IMPLEMENTATION


static void zglfw_default_error_callback(int error, const char *description) {
   fprintf(stderr, "Error: %s\n", description);
}

ZGLFWDEF void zglfw_init(ZGLFW *glfw) {

   if (!glfw->error_callback) {
      glfw->error_callback = zglfw_default_error_callback;
   }
   glfwSetErrorCallback(glfw->error_callback);

	if (!glfwInit()) {
      exit(EXIT_FAILURE);
   }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glfw->major_version);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glfw->minor_version);
   glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glfw->window = glfwCreateWindow(glfw->window_width, glfw->window_height, glfw->window_title, NULL, NULL);
	if (!glfw->window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

   if (glfw->key_callback) {
		glfwSetKeyCallback(glfw->window, glfw->key_callback);
   }

   if (glfw->window_size_callback) {
		glfwSetWindowSizeCallback(glfw->window, glfw->window_size_callback);
   }

	glfwSetInputMode(glfw->window, GLFW_CURSOR, glfw->window_cursor_mode);

	glfwMakeContextCurrent(glfw->window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);

   //glfwShowWindow(glfw->window);

	printf("OpenGL Loaded\n");
	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));

	glfw->current_time = glfwGetTime();
	glfw->last_time = glfw->current_time;
	glfw->delta_time = 0.0;

}

ZGLFWDEF void zglfw_show_window(ZGLFW *glfw) {
	glfwShowWindow(glfw->window);
}

ZGLFWDEF int zglfw_is_running(ZGLFW *glfw) {
   int should_close = glfw->should_close || glfwWindowShouldClose(glfw->window);
   return !should_close;
}

ZGLFWDEF void zglfw_begin(ZGLFW *glfw) {

	glfw->last_time = glfw->current_time;
	glfw->current_time = glfwGetTime();
	glfw->delta_time = glfw->current_time - glfw->last_time;

	for (int i = 32; i < GLFW_KEY_LAST; ++i) {
		if (glfwGetKey(glfw->window, i) == GLFW_PRESS) {
			glfw->keys[i] += 1;
		} else {
			glfw->keys[i] = 0;
		}
	}

	double x, y;
	glfwGetCursorPos(glfw->window, &x, &y);

	glfw->last_mouse_x = glfw->mouse_x;
	glfw->last_mouse_y = glfw->mouse_y;
	glfw->mouse_x = cast(float)x;
	glfw->mouse_y = cast(float)y;
	glfw->mouse_dx = glfw->mouse_x - glfw->last_mouse_x; 
	glfw->mouse_dy = glfw->mouse_y - glfw->last_mouse_y; 

   glClear(GL_COLOR_BUFFER_BIT);

   int r = glfw->window_background & 0xFF;
   int g = (glfw->window_background >> 8) & 0xFF;
   int b = (glfw->window_background >> 16) & 0xFF;
   int a = (glfw->window_background >> 24) & 0xFF;
   glClearColor(r / 255.99f, g / 255.99f, b / 255.99f, a / 255.99f);

   glfwGetFramebufferSize(glfw->window, &glfw->window_width, &glfw->window_height);
	glfw->window_widthf = glfw->window_width;
	glfw->window_heightf = glfw->window_height;

}

ZGLFWDEF void zglfw_end(ZGLFW *glfw) {
   glfwPollEvents();
   glfwSwapBuffers(glfw->window);
}

ZGLFWDEF void zglfw_quit(ZGLFW *glfw) {
	glfwDestroyWindow(glfw->window);
	glfwTerminate();
}

#endif // ZEN_GLFW_IMPLEMENTATION

