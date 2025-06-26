
#include <toytk/widget/frame.hxx>

namespace toytk
{
    Frame::Frame(bool interactive)
    {
	if (interactive)
	{
	    add_event_handler(frame_event_handler);
	}
    }

    void Frame::layout()
    {
	if (!m_content)
	{
	    return;
	}

	auto pos = get_content_position();
	m_content->set_position(pos);
	auto dim = get_content_dimension();
	m_content->set_dimension(dim);
    }

    void Frame::paint(cairo_t *cr) const
    {
	cairo_set_source_rgb(cr, frame_color_level, frame_color_level, frame_color_level);
	auto pos = get_position();
	auto dim = get_dimension();
	cairo_rectangle(cr, pos.x, pos.y, dim.width, dim.height);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, border_color_level, border_color_level, border_color_level);
	cairo_rectangle(cr, pos.x, pos.y, dim.width, dim.height);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);

	auto cpos = get_content_position();
	auto cdim = get_content_dimension();
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, cpos.x, cpos.y, cdim.width, cdim.height);
	cairo_fill(cr);

	if (!m_font)
	{
	    return;
	}

	auto extents = m_font.get_extents(cr, m_title.c_str());
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_move_to(cr, pos.x + border_width, pos.y + border_width - extents.y_bearing);
	m_font.show(cr, m_title.c_str());
	cairo_fill(cr);
    }

    Dimension Frame::get_preferred_dimension() const
    {
	if (m_content)
	{
	    auto cdim = m_content->get_preferred_dimension();
	    return Dimension {
		border_width * 2 + cdim.width,
		border_width + title_bar_width + cdim.height
	    };
	}
	else
	{
	    return Dimension {
		border_width * 2 + 1,
		border_width + title_bar_width + 1
	    };
	}
    }

    void Frame::for_each_child(void (*func)(Widget &, void *), void *data)
    {
	if (m_content)
	{
	    func(*m_content, data);
	}
    }

    std::optional<std::reference_wrapper<Widget>> Frame::get_content() const
    {
	if (m_content)
	{
	    return *m_content;
	}
	else
	{
	    return std::nullopt;
	}
    }

    void Frame::set_content(PmrPtr<Widget> &&widget)
    {
	reset_content();

	m_content = std::move(widget);
	own_child(m_content);
    }

    void Frame::reset_content()
    {
	if (m_content)
	{
	    unown_child(m_content);
	    m_content.reset();
	}
    }

    const Font &Frame::get_font() const
    {
	return m_font;
    }

    void Frame::set_font(const Font &font)
    {
	m_font = font;
	m_font.set_size(title_font_size);
    }

    const std::string &Frame::get_title() const
    {
	return m_title;
    }

    void Frame::set_title(std::string_view title)
    {
	m_title = title;
    }

    Position Frame::get_content_position() const
    {
	return Position { border_width, title_bar_width };
    }

    Dimension Frame::get_content_dimension() const
    {
	auto dim = get_dimension();
	return Dimension {
	    dim.width - border_width * 2,
	    dim.height - border_width - title_bar_width
	};
    }

    bool Frame::is_title_bar(double x, double y)
    {
	auto pos = get_position();
	auto dim = get_dimension();
	return pos.x <= x
	    && x <= pos.x + dim.width
	    && pos.y <= y
	    && y <= pos.y + title_bar_width;
    }

    bool Frame::is_top_resize_area(double x, double y)
    {
	auto pos = get_position();
	auto dim = get_dimension();
	return pos.x <= x
	    && x <= pos.x + dim.width
	    && pos.y <= y
	    && y <= pos.y + border_width;
    }

    bool Frame::is_bottom_resize_area(double x, double y)
    {
	auto pos = get_position();
	auto dim = get_dimension();
	return pos.x <= x
	    && x <= pos.x + dim.width
	    && pos.y + dim.height - border_width <= y
	    && y <= pos.y + dim.height;
    }

    bool Frame::is_left_resize_area(double x, double y)
    {
	auto pos = get_position();
	auto dim = get_dimension();
	return pos.x <= x
	    && x <= pos.x + border_width
	    && pos.y <= y
	    && y <= pos.y + dim.height;
    }

    bool Frame::is_right_resize_area(double x, double y)
    {
	auto pos = get_position();
	auto dim = get_dimension();
	return pos.x + dim.width - border_width <= x
	    && x <= pos.x + dim.width
	    && pos.y <= y
	    && y <= pos.y + dim.height;
    }

    Bubbling Frame::frame_event_handler(Widget &widget, Window &win, const Event &event)
    {
	Frame &this_ = *dynamic_cast<Frame *>(&widget);

	auto p = std::get_if<ButtonDownEvent>(&event);

	if (!p || p->button != BTN_LEFT)
	{
	    return Bubbling::bubble;
	}

	int edges = (this_.is_top_resize_area(p->x, p->y) ? Window::edges_top : 0)
	    | (this_.is_bottom_resize_area(p->x, p->y) ? Window::edges_bottom : 0)
	    | (this_.is_left_resize_area(p->x, p->y) ? Window::edges_left : 0)
	    | (this_.is_right_resize_area(p->x, p->y) ? Window::edges_right : 0);

	if (edges)
	{
	    win.resize(p->serial, edges);
	}
	else if (this_.is_title_bar(p->x, p->y))
	{
	    win.move(p->serial);
	}

	return Bubbling::stop;
    }
}
