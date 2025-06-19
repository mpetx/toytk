#ifndef INCLUDE_TOYTK_FONT_HXX_Z6WA4LDE
#define INCLUDE_TOYTK_FONT_HXX_Z6WA4LDE

#include <cairo/cairo.h>

#include <memory>

namespace toytk
{
    class Font
    {
	cairo_font_face_t *m_face;
	double m_size;

    public:

	Font();
	Font(cairo_font_face_t *, double);

	Font(const Font &);
	Font(Font &&);

	Font &operator=(const Font &);
	Font &operator=(Font &&);

	~Font();

	cairo_font_face_t *get_face() const;

	double get_size() const;
	void set_size(double);

	operator bool() const;

	void reset();

	void show(cairo_t *, const char *) const;

	cairo_text_extents_t get_extents(cairo_t *, const char *) const;
    };
}

#endif
