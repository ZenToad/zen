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
   int window_height;
   int window_cursor_mode;
   int window_background;
   int should_close;

   GLFWwindow* window;
   void (*error_callback)(int, const char*);
   void (*key_callback)(GLFWwindow*, int, int, int, int);
   void (*window_size_callback)(GLFWwindow*, int, int);

} ZGLFW;

ZGLFWDEF void zglfw_init(ZGLFW *glfw);
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

static void zglfw_default_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	printf("key: %d, scancode: %d, action: %d, mods: %d\n", key, scancode, action, mods);

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

}

static void zglfw_default_window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
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

	glfw->window = glfwCreateWindow(glfw->window_width, glfw->window_height, glfw->window_title, NULL, NULL);
	if (!glfw->window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

   if (!glfw->key_callback) {
      glfw->key_callback = zglfw_default_key_callback;
   }
   glfwSetKeyCallback(glfw->window, glfw->key_callback);

   if (!glfw->window_size_callback) {
      glfwSetWindowSizeCallback(glfw->window, glfw->window_size_callback);
   }
   glfwSetWindowSizeCallback(glfw->window, glfw->window_size_callback);

	//glfwSetCharCallback(window, character_callback);
	//glfwSetCursorPosCallback(window, cursor_position_callback);
	//glfwSetCursorEnterCallback(window, cursor_enter_callback);

	glfwSetInputMode(glfw->window, GLFW_CURSOR, glfw->window_cursor_mode);

	glfwMakeContextCurrent(glfw->window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);

   glfwShowWindow(glfw->window);

	printf("OpenGL Loaded\n");
	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));
}

ZGLFWDEF int zglfw_is_running(ZGLFW *glfw) {
   int should_close = glfw->should_close || glfwWindowShouldClose(glfw->window);
   return !should_close;
}

ZGLFWDEF void zglfw_begin(ZGLFW *glfw) {

   glClear(GL_COLOR_BUFFER_BIT);

   int r = glfw->window_background & 0xFF;
   int g = (glfw->window_background >> 8) & 0xFF;
   int b = (glfw->window_background >> 16) & 0xFF;
   int a = (glfw->window_background >> 24) & 0xFF;
   glClearColor(r / 255.99f, g / 255.99f, b / 255.99f, a / 255.99f);

   glfwGetFramebufferSize(glfw->window, &glfw->window_width, &glfw->window_height);

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

