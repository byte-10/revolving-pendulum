#include "gl_debug.h"

#define CASE_TO_STR(constant) case constant: return #constant;

const char* gl_error_enum_to_string(GLenum error)
{
  switch (error) {
    CASE_TO_STR(GL_NO_ERROR)
    CASE_TO_STR(GL_INVALID_ENUM)
    CASE_TO_STR(GL_INVALID_VALUE)
    CASE_TO_STR(GL_INVALID_OPERATION)
    CASE_TO_STR(GL_INVALID_FRAMEBUFFER_OPERATION)
    CASE_TO_STR(GL_OUT_OF_MEMORY)
  default:
    return "Unknown GL error";
  }
  return nullptr;
}