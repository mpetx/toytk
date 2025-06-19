
#include <toytk/widget/label.hxx>

namespace toytk
{
    Label::Label()
    {
    }

    void Label::layout()
    {
    }

    void Label::paint(cairo_t *cr) const
    {
	if (!m_font)
	{
	    return;
	}

	auto extents = m_font.get_extents(cr, m_text.c_str());
	auto pos = get_position();
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_move_to(cr, pos.x, pos.y - extents.y_bearing);
	m_font.show(cr, m_text.c_str());
	cairo_fill(cr);
    }

    Dimension Label::get_preferred_dimension() const
    {
	if (m_font)
	{
	    detail::CairoSurfacePtr surface { cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1) };
	    detail::CairoPtr cr { cairo_create(surface.get()) };
	    auto extents = m_font.get_extents(cr.get(), m_text.c_str());
	    return Dimension {
		static_cast<std::int32_t>(extents.width),
		static_cast<std::int32_t>(extents.height)
	    };
	}
	else
	{
	    return Dimension { 1, 1 };
	}
    }

    void Label::for_each_child(void (*)(Widget &, void *), void *)
    {
    }

    const std::string &Label::get_text() const
    {
	return m_text;
    }

    void Label::set_text(std::string_view text)
    {
	m_text = text;
    }

    const Font &Label::get_font() const
    {
	return m_font;
    }

    void Label::set_font(const Font &font)
    {
	m_font = font;
    }
}
