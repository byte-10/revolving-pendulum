#pragma once
#include <SDL_stdinc.h>
#include <SDL_opengl.h>
#include "stb_truetype.h"

enum {
	LF_X=1<<0,
	LF_Y=1<<1,
};

class gl_font
{
	int             m_resetmode;
	float           m_curr_x;
	float           m_curr_y;
	float           m_coordx;
	float           m_coordy;
	float           m_font_width;
	float           m_color[4];
	GLuint          m_font_tex;
	stbtt_bakedchar m_chars_data[96]{};
public:
	gl_font();

	void move_to(float x, float y);
	void move_rel_to(float xr, float yr);
	void line_feed_mode(int flags);
	void reset(int flags); //TR_X or TR_Y
	void set_fcolor(float r, float g, float b, float a);
	void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	bool load_ttf(const char *pfilename, float font_size = 32.f);
	bool load_ttf_raw(Uint8 *psrc, Uint32 size, float font_size=32.f);
	void draw_textf(const char* pformat, ...);
	void draw_text(const char* ptext);
	inline float get_font_width() const { return m_font_width; }

	/* drawing text begin/end */
	static void begin_text(int width, int height);
	static void end_text();
};