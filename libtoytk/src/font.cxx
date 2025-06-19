
#include <toytk/font.hxx>

namespace toytk
{
    Font::Font()
	: m_face { nullptr },
	  m_size { 0 }
    {
    }

    Font::Font(cairo_font_face_t *face, double size)
	: m_face { face },
	  m_size { size }
    {
	cairo_font_face_reference(face);
    }

    Font::Font(const Font &font)
	: m_face { font.m_face },
	  m_size { font.m_size }
    {
	cairo_font_face_reference(m_face);
    }

    Font::Font(Font &&font)
	: m_face { font.m_face },
	  m_size { font.m_size }
    {
	font.m_face = nullptr;
	font.m_size = 0;
    }

    Font &Font::operator=(const Font &font)
    {
	if (this == &font)
	{
	    return *this;
	}

	reset();

	m_face = font.m_face;
	m_size = font.m_size;

	cairo_font_face_reference(m_face);

	return *this;
    }

    Font &Font::operator=(Font &&font)
    {
	if (this == &font)
	{
	    return *this;
	}

	reset();

	m_face = font.m_face;
	m_size = font.m_size;

	font.m_face = nullptr;
	font.m_size = 0;

	return *this;
    }

    Font::~Font()
    {
	reset();
    }

    cairo_font_face_t *Font::get_face() const
    {
	return m_face;
    }

    double Font::get_size() const
    {
	return m_size;
    }

    void Font::set_size(double size)
    {
	m_size = size;
    }

    Font::operator bool() const
    {
	return static_cast<bool>(m_face);
    }

    void Font::reset()
    {
	if (!m_face)
	{
	    return;
	}

	cairo_font_face_destroy(m_face);

	m_face = nullptr;
	m_size = 0;
    }

    void Font::show(cairo_t *cr, const char *text) const
    {
	if (!m_face)
	{
	    return;
	}

	cairo_set_font_face(cr, m_face);
	cairo_set_font_size(cr, m_size);
	cairo_show_text(cr, text);
    }

    cairo_text_extents_t Font::get_extents(cairo_t *cr, const char *text) const
    {
	cairo_set_font_face(cr, m_face);
	cairo_set_font_size(cr, m_size);

	cairo_text_extents_t extents;
	cairo_text_extents(cr, text, &extents);

	return extents;
    }
}
