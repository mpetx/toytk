
#include <toytk/widget/button.hxx>

namespace toytk
{
    Button::Button()
    {
	add_event_handler(button_event_handler);
    }

    void Button::layout()
    {
    }

    void Button::paint(cairo_t *cr) const
    {
	if (is_hovered())
	{
	    cairo_set_source_rgb(cr, hover_color_level, hover_color_level, hover_color_level);
	}
	else
	{
	    cairo_set_source_rgb(cr, normal_color_level, normal_color_level, normal_color_level);
	}

	auto pos = get_position();
	auto dim = get_dimension();
	cairo_rectangle(cr, pos.x, pos.y, dim.width, dim.height);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, border_color_level, border_color_level, border_color_level);
	cairo_rectangle(cr, pos.x, pos.y, dim.width, dim.height);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);

	auto extents = m_font.get_extents(cr, m_text.c_str());
	auto text_x = pos.x + (dim.width - extents.width) / 2.;
	auto text_y = pos.y + (dim.height - extents.height) / 2. - extents.y_bearing;
	cairo_move_to(cr, text_x, text_y);
	cairo_set_source_rgb(cr, 0, 0, 0);
	m_font.show(cr, m_text.c_str());
	cairo_fill(cr);
    }

    Dimension Button::get_preferred_dimension() const
    {
	if (!m_font)
	{
	    return Dimension { 2 * preferred_x_padding, 2 * preferred_y_padding };
	}

	detail::CairoSurfacePtr surface { cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1) };
	detail::CairoPtr cr { cairo_create(surface.get()) };

	auto extents = m_font.get_extents(cr.get(), m_text.c_str());

	return Dimension {
	    2 * preferred_x_padding + static_cast<std::int32_t>(extents.width),
	    2 * preferred_y_padding + static_cast<std::int32_t>(extents.height)
	};
    }

    void Button::for_each_child(void (*)(Widget &, void *), void *)
    {
    }

    const std::string &Button::get_text() const
    {
	return m_text;
    }

    void Button::set_text(std::string_view text)
    {
	m_text = text;
    }

    const Font &Button::get_font() const
    {
	return m_font;
    }

    void Button::set_font(const Font &font)
    {
	m_font = font;
    }

    Bubbling Button::button_event_handler(Widget &widget, Window &win, const Event &event)
    {
	if (std::get_if<PointerEnterEvent>(&event)
	    || std::get_if<PointerLeaveEvent>(&event))
	{
	    win.set_redraw_needed();
	}

	return Bubbling::bubble;
    }
}
