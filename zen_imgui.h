#ifndef __ZEN_IMGUI_H__
#define __ZEN_IMGUI_H__

#ifdef ZEN_IMGUI_STATIC
#define ZIMGUIDEF static
#else
#define ZIMGUIDEF extern
#endif

#if defined(ZEN_LIB_DEV)
#include "zen_glfw.h"
#endif

ZIMGUIDEF void zen_imgui_init(ZGLFW *glfw);
ZIMGUIDEF void zen_imgui_begin(ZGLFW *glfw);
ZIMGUIDEF void zen_imgui_end();
ZIMGUIDEF void zen_imgui_quit();


#endif //__ZEN_IMGUI_H__




#if defined(ZEN_IMGUI_IMPLEMENTATION) || defined(ZEN_LIB_DEV)

#if defined(ZEN_LIB_DEV)
#include "zen_gl.h"
#include "zen_glfw.h"
#include "imgui.h"
#endif
  
typedef struct ZenImguiState_t {

	GLuint vbo;
	GLuint ebo;
	GLuint vao;

	ZGLShader zen_imgui_shader;

} ZenImguiState_t;

static ZenImguiState_t __zen_imgui_state = {0};

static ImFont* __zen_imgui_default_font;
static ImFont* __zen_imgui_custion_font;


ZIMGUIDEF void zen_imgui_begin(ZGLFW *glfw) {

	ImGuiIO& io = ImGui::GetIO();
	
	// Setup display size (every frame to accommodate for window resizing)
	int w = glfw->window_width;
	int h = glfw->window_height;
	int display_w = glfw->display_width;
	int display_h = glfw->display_height;

	io.DisplaySize = ImVec2((float)w, (float)h);
	io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

	io.DeltaTime = glfw->delta_time;

	// Setup inputs
	// (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
	// Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
	if (glfwGetWindowAttrib(glfw->window, GLFW_FOCUSED)) {
		io.MousePos = ImVec2(glfw->mouse_x, glfw->mouse_y);   
	} else {
		io.MousePos = ImVec2(-1,-1);
	}

	for (int i = 0; i < 3; i++) {
		io.MouseDown[i] = glfw->mouse_button[i] ? true : false;
	}

	io.MouseWheel = glfw->mouse_scroll;

	for (int i = 32; i < GLFW_KEY_LAST; ++i) {
		io.KeysDown[i] = glfw->keys[i] ? true : false;
	}


	// Hide OS mouse cursor if ImGui is drawing it
	glfwSetInputMode(glfw->window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

	// Start the frame
	ImGui::NewFrame();

}

ZIMGUIDEF void zen_imgui_end() {
	ImGui::Render();
}

static const char* zen_imgui_default_get_clipboard_callback(void* user_data) {
	return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void zen_imgui_default_set_clipboard_callback(void* user_data, const char* text) {
	glfwSetClipboardString((GLFWwindow*)user_data, text);
}

ZIMGUIDEF void zen_imgui_quit() {
	ImGui::Shutdown();
}

static void zen_imgui_create_font() {
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	// Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely 
	// to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept 
	// than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
	__zen_imgui_default_font = io.Fonts->AddFontDefault();
	__zen_imgui_custion_font = io.Fonts->AddFontFromFileTTF("res/PressStart2P.ttf", 16);

	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   

	// Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glActiveTexture(GL_TEXTURE0 + 1);
	GLuint font_texture;
	glGenTextures(1, &font_texture);
	glBindTexture(GL_TEXTURE_2D, font_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)font_texture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);

}

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
ZIMGUIDEF void zen_imgui_render(ImDrawData* draw_data) {

	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0)
		return;
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Backup GL state
	GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
	GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
	GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
	GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
	GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
	GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
	GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box); 
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0 + 1);

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	const float ortho_projection[4][4] =
	{
		{ 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
		{ 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
		{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
		{-1.0f,                  1.0f,                   0.0f, 1.0f },
	};

	ZGLShader *shader = &__zen_imgui_state.zen_imgui_shader;
	ZenImguiState_t *state = &__zen_imgui_state;
	glUseProgram(shader->program);
	zgl_set_uniform_int(shader, "Texture", 1);
	zgl_set_uniform_mat4(shader, "ProjMtx", &ortho_projection[0][0]);

	glBindVertexArray(state->vao);

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}
			idx_buffer_offset += pcmd->ElemCount;
		}
	}

	// Restore modified GL state
	glUseProgram(last_program);
	glActiveTexture(last_active_texture);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindVertexArray(last_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFunc(last_blend_src, last_blend_dst);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

//bool ImGui_ImplGlfwGL3_CreateDeviceObjects()
static void zen_imgui_setup_shaders() {

	GLint last_texture, last_array_buffer, last_vertex_array;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

	const GLchar *vertex_shader =
		"#version 420 core\n"
		"uniform mat4 ProjMtx;\n"
		"in vec2 Position;\n"
		"in vec2 UV;\n"
		"in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"	Frag_UV = UV;\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"#version 420 core\n"
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
		"}\n";

	ZGLShaderError result = zgl_create_shader(&__zen_imgui_state.zen_imgui_shader, vertex_shader, fragment_shader);

	if (result != ZGL_ERROR_NONE) {
		exit(EXIT_FAILURE);
	}

	ZGLShader *shader = &__zen_imgui_state.zen_imgui_shader;
	glUseProgram(shader->program);

	glGenVertexArrays(1, &__zen_imgui_state.vao);
	glBindVertexArray(__zen_imgui_state.vao);
	__zen_imgui_state.vbo = zgl_make_vbo(NULL, 0, GL_STREAM_DRAW);
	__zen_imgui_state.ebo = zgl_make_ebo(NULL, 0, GL_STREAM_DRAW);
	zgl_bind_vbo(__zen_imgui_state.vbo);

	int32 loc = glGetAttribLocation(shader->program, "Position");
	assert(loc != -1 && "Position");
	zgl_vert_ptr_aa(loc, 2, ImDrawVert, pos);
	loc = glGetAttribLocation(shader->program, "UV");
	assert(loc != -1 && "UV");
	zgl_vert_ptr_aa(loc, 2, ImDrawVert, uv);
	loc = glGetAttribLocation(shader->program, "Color");
	assert(loc != -1 && "Color");
	zgl_vert_ptr_aa_u8n(loc, 4, ImDrawVert, col);

	zen_imgui_create_font();

	// Restore modified GL state
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindVertexArray(last_vertex_array);

}

ZIMGUIDEF void zen_imgui_init(ZGLFW *glfw) {

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                         
	io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	// Alternatively you can set this to NULL and call ImGui::GetDrawData() 
	// after ImGui::Render() to get the same ImDrawData pointer.
	io.RenderDrawListsFn = zen_imgui_render;       
	io.SetClipboardTextFn = zen_imgui_default_set_clipboard_callback;
	io.GetClipboardTextFn = zen_imgui_default_get_clipboard_callback;
	io.ClipboardUserData = glfw->window;
#ifdef _WIN32
	io.ImeWindowHandle = glfwGetWin32Window(glfw->window);
#endif

	zen_imgui_setup_shaders();
}

#endif //ZEN_IMGUI_IMPLEMENTATION




