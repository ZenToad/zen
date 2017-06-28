/* zen_sdl.h - v0.42 - SDL wrapper -https://github.com/ZenToad/zen

   Do this:
      #define ZEN_SDL_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define ZEN_SDL_IMPLEMENTATION
   #include "zen_sdl.h"

	 zlib license:
	 Full license at bottom of file.

*/

#if !defined(__ZEN_SDL_H__)
#define __ZEN_SDL_H__


#if defined(__cplusplus)
extern "C" {
#endif


#if defined(ZEN_SDL_STATIC)
#define ZSDL_DEF static
#else
#define ZSDL_DEF extern
#endif


#if defined(ZEN_LIB_DEV)
#include "SDL.h"
#include "glad/glad.h"
#include "zen/zen_lib.h"
#endif


#define KEY_DOWN(SDL,KEY) SDL->keys[SDL_SCANCODE_##KEY]
#define KEY_DOWN_ONCE(SDL,KEY) SDL->keys[SDL_SCANCODE_##KEY] == 1

#define MOUSE_DOWN(SDL,B) SDL->mouse_button[(SDL_BUTTON_##B)-1]
#define MOUSE_DOWN_ONCE(SDL,B) SDL->mouse_button[(SDL_BUTTON_##B)-1] == 1

typedef struct zen_sdl {

	enum { 
		ZEN_SDL_KEY_LAST = SDL_SCANCODE_APP2,
		SDL_MOUSE_COUNT = 3
	};
	const char *window_title;

   int major_version;
   int minor_version;
   int window_cursor_mode;
   int window_background;
	int should_close;
   int got_quit_message;

	int window_width;
	int window_height;
	float window_widthf;
	float window_heightf;

	int display_width;
	int display_height;
	float display_widthf;
	float display_heightf;

	double current_time;
	double last_time;
	double delta_time;
	double total_time;
	int total_frames;

	float mouse_x;
	float mouse_y;
	float mouse_dx;
	float mouse_dy;
	float last_mouse_x;
	float last_mouse_y;
	float mouse_scroll;
	int mouse_button[SDL_MOUSE_COUNT];

   SDL_Window* window;
	int keys[ZEN_SDL_KEY_LAST];

	void (*window_event_callback)(zen_sdl *sdl, SDL_WindowEvent *e);
	void (*text_input_callback)(zen_sdl *sdl, SDL_TextInputEvent *e);
	void (*keyboard_callback)(zen_sdl *sdl, SDL_KeyboardEvent *e);
	void (*mouse_wheel_callback)(zen_sdl *sdl, SDL_MouseWheelEvent *e);
	void (*mouse_button_callback)(zen_sdl *sdl, SDL_MouseButtonEvent *e);
	void (*mouse_motion_callback)(zen_sdl *sdl, SDL_MouseMotionEvent *e);

} zen_sdl;


ZSDL_DEF zen_sdl *zen_sdl_create(const char *title, int width, int height, int major = 4, int minor = 5);
ZSDL_DEF void zen_sdl_init(zen_sdl *sdl);
ZSDL_DEF void zen_sdl_show_window(zen_sdl *sdl);
ZSDL_DEF int zen_sdl_is_running(zen_sdl *sdl);
ZSDL_DEF void zen_sdl_begin(zen_sdl *sdl);
ZSDL_DEF void zen_sdl_end(zen_sdl *sdl);
ZSDL_DEF void zen_sdl_quit(zen_sdl *sdl);
ZSDL_DEF void zen_sdl_destroy(zen_sdl *sdl);

ZSDL_DEF uint8 *zen_sdl_load_file(const char *path, isize *size = NULL);


#if defined(__cplusplus)
}
#endif

#endif // __ZEN_SDL_H__


//------------------------------------------
//
// Implementation
//
//------------------------------------------
#if defined(ZEN_SDL_IMPLEMENTATION)


void zen_sdl_default_window_event_callback(zen_sdl *sdl, SDL_WindowEvent* e) {
	//if ()
	//glViewport(0, 0, e->data1, e->data2);
}


void zen_sdl_default_text_input_callback(zen_sdl *sdl, SDL_TextInputEvent *e) {
	// do nothing on default	
}

void zen_sdl_default_keyboard_callback(zen_sdl *sdl, SDL_KeyboardEvent *e) {

	int key = e->keysym.scancode;
	if (e->type == SDL_KEYDOWN) {
		sdl->keys[key]++;
	} else if (e->type == SDL_KEYUP) {
		sdl->keys[key] = 0;
	}

}

void zen_sdl_default_mouse_wheel_callback(zen_sdl *sdl, SDL_MouseWheelEvent *e) {
	sdl->mouse_scroll += e->y;
}

void zen_sdl_default_mouse_button_callback(zen_sdl * sdl, SDL_MouseButtonEvent *e) {

	int button = (e->button - 1);
	if (button < 3) {
		if (e->type == SDL_MOUSEBUTTONDOWN) {
			sdl->mouse_button[button]++;
		} else if (e->type == SDL_MOUSEBUTTONUP) {
			sdl->mouse_button[button] = 0;
		}
	}

}

void zen_sdl_default_mouse_motion_callback(zen_sdl *sdl, SDL_MouseMotionEvent *e) {

	sdl->mouse_x = e->x;
	sdl->mouse_y = e->y;
	sdl->mouse_dx = e->xrel;
	sdl->mouse_dy = e->yrel;

}

static void zen_sdl_die(const char * message) {
	zout("%s: %s\n", message, SDL_GetError());
	exit(2);
}


ZSDL_DEF zen_sdl *zen_sdl_create(const char *title, int width, int height, int major, int minor) {

	zen_sdl *sdl = ZEN_CALLOC(zen_sdl, 1);
	GB_ASSERT_NOT_NULL(sdl);

	sdl->window_title = title;
	sdl->major_version = major;
	sdl->minor_version = minor;
	sdl->window_width = width;
	sdl->window_height = height;
	sdl->window_background = 0xFF333333;

	return sdl;
}


ZSDL_DEF void zen_sdl_destroy(zen_sdl *sdl) {
	free(sdl);
}


void zen_sdl_init(zen_sdl *sdl) {

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		zen_sdl_die("Couldn't initialize SDL");

	atexit (SDL_Quit);
	SDL_GL_LoadLibrary(NULL); // Default OpenGL is fine.

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, sdl->major_version);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, sdl->minor_version);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create the window
	sdl->window = SDL_CreateWindow( sdl->window_title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		sdl->window_width, sdl->window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL
	);
	if (sdl->window == NULL)
		zen_sdl_die("Couldn't set video mode");

	SDL_GLContext maincontext = SDL_GL_CreateContext(sdl->window);
	if (maincontext == NULL)
		zen_sdl_die("Failed to create OpenGL context");

	// Check OpenGL properties
	printf("OpenGL loaded\n");
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));

	GB_ASSERT(GLVersion.major >= sdl->major_version);
	GB_ASSERT(GLVersion.minor >= sdl->minor_version);

	// Use v-sync
	SDL_GL_SetSwapInterval(1);

	sdl->should_close = 0;


	// callbacks
	if (sdl->window_event_callback == NULL) {
		sdl->window_event_callback = zen_sdl_default_window_event_callback;
	}

	if (sdl->text_input_callback == NULL) {
		sdl->text_input_callback = zen_sdl_default_text_input_callback;
	}

	if (sdl->keyboard_callback == NULL) {
		sdl->keyboard_callback = zen_sdl_default_keyboard_callback;
	}

	if (sdl->mouse_wheel_callback == NULL) {
		sdl->mouse_wheel_callback = zen_sdl_default_mouse_wheel_callback;
	}

	if (sdl->mouse_button_callback == NULL) {
		sdl->mouse_button_callback = zen_sdl_default_mouse_button_callback;
	}

	if (sdl->mouse_motion_callback == NULL) {
		sdl->mouse_motion_callback = zen_sdl_default_mouse_motion_callback;
	}

	for (int i = 0; i < sdl->ZEN_SDL_KEY_LAST; ++i) {
		sdl->keys[i] = 0;
	}

	sdl->current_time = cast(double)SDL_GetPerformanceCounter() / cast(double)SDL_GetPerformanceFrequency();
	sdl->last_time = sdl->current_time;
	sdl->delta_time = 0.0;
	sdl->total_time = 0;
	sdl->total_frames = 0;

	SDL_StartTextInput();

}


ZSDL_DEF void zen_sdl_show_window(zen_sdl *sdl) {
	SDL_ShowWindow(sdl->window);
}


ZSDL_DEF int zen_sdl_is_running(zen_sdl *sdl) {
	return !(sdl->should_close || sdl->got_quit_message);
}


ZSDL_DEF void zen_sdl_begin(zen_sdl *sdl) {

	// do time here
	sdl->last_time = sdl->current_time;
	sdl->current_time = cast(double)SDL_GetPerformanceCounter() / cast(double)SDL_GetPerformanceFrequency();
	sdl->delta_time = sdl->current_time - sdl->last_time;
	sdl->total_time += sdl->delta_time;
	sdl->total_frames++;


	sdl->last_mouse_x = sdl->mouse_x;
	sdl->last_mouse_y = sdl->mouse_y;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT: {
				sdl->got_quit_message = 1;
			} break;
			case SDL_KEYDOWN:
			case SDL_KEYUP: {
				sdl->keyboard_callback(sdl, &event.key);
			} break;
			case SDL_TEXTINPUT: {
			   sdl->text_input_callback(sdl, &event.text);
			} break;
			case SDL_MOUSEWHEEL: {
				sdl->mouse_wheel_callback(sdl, &event.wheel);
			} break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP: {
				sdl->mouse_button_callback(sdl, &event.button);
			} break;
			case SDL_MOUSEMOTION: {
				sdl->mouse_motion_callback(sdl, &event.motion);
			} break;
			case SDL_WINDOWEVENT: {
				sdl->window_event_callback(sdl, &event.window);
			} break;
		}
	}


   int r = sdl->window_background & 0xFF;
   int g = (sdl->window_background >> 8) & 0xFF;
   int b = (sdl->window_background >> 16) & 0xFF;
   int a = (sdl->window_background >> 24) & 0xFF;
   glClearColor(r / 255.99f, g / 255.99f, b / 255.99f, a / 255.99f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}


ZSDL_DEF void zen_sdl_end(zen_sdl *sdl) {

	for (int i = 0; i < sdl->ZEN_SDL_KEY_LAST; ++i) {
		if (sdl->keys[i]) {
			sdl->keys[i]++;
		} 
	}

	for (int i = 0; i < sdl->SDL_MOUSE_COUNT; ++i) {
		if (sdl->mouse_button[i]) 
			sdl->mouse_button[i]++;
	}

	sdl->mouse_scroll = 0;
	SDL_GL_SwapWindow(sdl->window);

}


ZSDL_DEF void zen_sdl_quit(zen_sdl *sdl) {
	int ms = (int)(1.0e3 * sdl->total_time / sdl->total_frames);
	zout("\n%d (ms)\n", ms);
	SDL_Quit();
}


ZSDL_DEF uint8 *zen_sdl_load_file(const char *path, isize *size) {

	SDL_RWops *rw = SDL_RWFromFile(path, "rb");
	if (rw == NULL) {
		zout("%s: %s", path, SDL_GetError());
		return NULL;
	}

	int64 res_size = SDL_RWsize(rw);
	uint8 *mem = ZEN_CALLOC(uint8, res_size + 1);
	GB_ASSERT_NOT_NULL(mem);

	int64 nb_read_total = 0;
	int64 nb_read = 1;
	uint8 *buf = mem;

	while (nb_read_total < res_size && nb_read != 0) {
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}

	SDL_RWclose(rw);
	if (nb_read_total != res_size) {
		free(mem);
		return NULL;
	}

	mem[nb_read_total] = '\0';
	if (size) *size = res_size;
	return mem;

}

#endif
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
