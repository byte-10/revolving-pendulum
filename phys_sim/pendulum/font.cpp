#include "font.h"
#include <stdio.h>
#include <SDL_log.h>
#include <gl/GLU.h>
#include <float.h>
#include <algorithm>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

void gl_font::draw_textf(const char* pformat, ...)
{
  char buf[1024];
  va_list argptr;
  va_start(argptr, pformat);
  SDL_vsnprintf(buf, sizeof(buf), pformat, argptr);
  va_end(argptr);
  draw_text(buf);
}

void gl_font::draw_text(const char* text)
{
  m_curr_y += m_font_width;
  // assume orthographic projection with units = screen pixels, origin at top left
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_font_tex);
  glBegin(GL_QUADS);
  glColor3fv(m_color);
  while (*text) {
    if (*text >= 32 && *text < 128) {
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(m_chars_data, 512, 512, *text - 32, &m_curr_x, &m_curr_y, &q, 1);//1=opengl & d3d10+,0=d3d9
      glTexCoord2f(q.s0, q.t0);
      glVertex2f(q.x0, q.y0);
      glTexCoord2f(q.s1, q.t0);
      glVertex2f(q.x1, q.y0);
      glTexCoord2f(q.s1, q.t1);
      glVertex2f(q.x1, q.y1);
      glTexCoord2f(q.s0, q.t1);
      glVertex2f(q.x0, q.y1);
    }
    ++text;
  }
  glEnd();

  if (m_resetmode)
    reset(m_resetmode);
}

//void gl_font::text_metric(metric& dst)
//{
//}

void gl_font::line_boundsf(float& dstx, float& dsty, const char* pformat, ...)
{
  char buf[512];
  char* ptr = buf;
  float x = 0, y = 0;
  float minx = FLT_MAX, miny = FLT_MAX;
  float maxx = -FLT_MAX, maxy = -FLT_MAX;
  va_list argptr;
  va_start(argptr, pformat);
  SDL_vsnprintf(buf, sizeof(buf), pformat, argptr);
  va_end(argptr);
  while (*ptr) {
    int c = *ptr;
    if (c < 32 || c >= 128)
      continue;

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(
      m_chars_data,
      512, 512,
      c - 32,
      &x, &y,
      &q,
      /*opengl_fillrule=*/1
    );
    minx = std::min(minx, q.x0);
    miny = std::min(miny, q.y0);
    maxx = std::max(maxx, q.x1);
    maxy = std::max(maxy, q.y1);
    ptr++;
  }

  if (minx == FLT_MAX)
    return;

  dstx = maxx - minx;
  dsty = maxy - miny;
}

void gl_font::begin_text(int width, int height)
{
  glPushAttrib(GL_CURRENT_BIT);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0., (double)width, (double)height, 0.);
  glMatrixMode(GL_MODELVIEW);
}

void gl_font::end_text()
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glPopAttrib();
}

gl_font::gl_font()
{
  m_resetmode = 0;
  m_curr_x = m_curr_y=0.f;
  m_coordx = m_coordy = 0.f;
  m_font_width = 32.f;
  m_color[0] = m_color[1] = m_color[2] = m_color[3] = 1.f;
  m_font_tex = 0;
}

void gl_font::move_to(float x, float y)
{
  m_curr_x = m_coordx = x;
  m_curr_y = m_coordy = y;
}

void gl_font::move_rel_to(float xr, float yr)
{
  m_curr_x += xr;
  m_curr_y += yr;
}

void gl_font::line_feed_mode(int flags)
{
  m_resetmode = flags;
}

void gl_font::reset(int flags)
{
  if (flags & LF_X)
    m_curr_x = m_coordx;
  if (flags & LF_Y)
    m_curr_y = m_coordy;
}

void gl_font::set_fcolor(float r, float g, float b, float a)
{
  m_color[0] = r;
  m_color[1] = g;
  m_color[2] = b;
  m_color[3] = a;
}

void gl_font::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  m_color[0] = r / 255.f;
  m_color[1] = g / 255.f;
  m_color[2] = b / 255.f;
  m_color[3] = a / 255.f;
}

bool gl_font::load_ttf(const char* pfilename, float font_size)
{
  static Uint8 tempbuf[1024 * 1024];
  FILE* fp;
#ifdef _MSC_VER
  fopen_s(&fp, pfilename, "rb");
#else
  fp = fopen(pfilename, "rb");
#endif
  if (!fp)
    return false;

  Uint32 size = (Uint32)fread(tempbuf, 1, sizeof(tempbuf), fp);
  fclose(fp);
  return load_ttf_raw(tempbuf, size, font_size);
}

bool gl_font::load_ttf_raw(Uint8* psrc, Uint32 size, float font_size)
{
  GLenum error;
  static unsigned char temp_bitmap[512 * 512];
  m_font_width = font_size;
  stbtt_BakeFontBitmap(psrc, 0, m_font_width, temp_bitmap, 512, 512, 32, 96, m_chars_data);

  glGenTextures(1, &m_font_tex);
  glBindTexture(GL_TEXTURE_2D, m_font_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
  error = glGetError();
  if (error != GL_NO_ERROR) {
    SDL_LogError(0, "glTexImage2D() returned error: %d (0x%x)", error, error);
    return false;
  }
  // can free temp_bitmap at this point
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  return true;
}
