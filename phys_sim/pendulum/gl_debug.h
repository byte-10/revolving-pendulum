#pragma once
#include <intrin.h> //__debugbreak
#include <SDL_opengl.h>

const char* gl_error_enum_to_string(GLenum error);

#ifdef _DEBUG
#define GL_CALL(func) do {\
	func;\
	GLenum gl_current_error = glGetError(); \
	if(gl_current_error != GL_NO_ERROR) { \
		fprintf(stderr, "[GL ERROR] Call " #func " failed with error (%s) code %d (0x%X)\n", gl_error_enum_to_string(gl_current_error), gl_current_error, gl_current_error); \
		__debugbreak(); \
	}\
} while(0.0);

#else
#define GL_CALL(func)
#endif

