
#include <toytk/widget/padding-box.hxx>

#include <algorithm>

namespace toytk
{
    PaddingBox::PaddingBox()
    {
    }

    void PaddingBox::layout()
    {
	if (!m_content.has_value())
	{
	    return;
	}

	auto pos = get_position();
	auto dim = get_dimension();

	double x_ratio = m_left_padding / (m_left_padding + m_right_padding + 0.);
	std::int32_t x_total_padding = std::min(m_left_padding + m_right_padding, dim.width);
	double y_ratio = m_top_padding / (m_top_padding + m_bottom_padding + 0.);
	std::int32_t y_total_padding = std::min(m_top_padding + m_bottom_padding, dim.height);

	Position cpos {
	    pos.x + static_cast<std::int32_t>(x_ratio * x_total_padding),
	    pos.y + static_cast<std::int32_t>(y_ratio * y_total_padding)
	};

	m_content->get().set_position(cpos);

	Dimension cdim {
	    std::max(dim.width - m_left_padding - m_right_padding, 1),
	    std::max(dim.height - m_top_padding - m_bottom_padding, 1)
	};

	m_content->get().set_dimension(cdim);
    }

    void PaddingBox::paint(cairo_t *cr) const
    {
	cairo_set_source_rgb(cr, 1, 1, 1);
	auto pos = get_position();
	auto dim = get_dimension();
	cairo_rectangle(cr, pos.x, pos.y, dim.width, dim.height);
	cairo_fill(cr);
    }

    Dimension PaddingBox::get_preferred_dimension() const
    {
	if (m_content.has_value())
	{
	    auto dim = m_content->get().get_preferred_dimension();
	    return Dimension {
		m_left_padding + m_right_padding + dim.width,
		m_top_padding + m_bottom_padding + dim.height
	    };
	}
	else
	{
	    return Dimension {
		m_left_padding + m_right_padding + 1,
		m_top_padding + m_bottom_padding + 1
	    };
	}
    }

    void PaddingBox::for_each_child(void (*func)(Widget &, void *), void *data)
    {
	if (m_content.has_value())
	{
	    func(m_content->get(), data);
	}
    }

    std::optional<std::reference_wrapper<Widget>> PaddingBox::get_content() const
    {
	return m_content;
    }

    void PaddingBox::set_content(Widget &widget)
    {
	reset_content();
	m_content = widget;
	widget.set_parent(*this);
    }

    void PaddingBox::reset_content()
    {
	if (m_content.has_value())
	{
	    m_content->get().reset_parent();
	    m_content = std::nullopt;
	}
    }

    std::int32_t PaddingBox::get_top_padding() const
    {
	return m_top_padding;
    }

    std::int32_t PaddingBox::get_bottom_padding() const
    {
	return m_bottom_padding;
    }

    std::int32_t PaddingBox::get_left_padding() const
    {
	return m_left_padding;
    }

    std::int32_t PaddingBox::get_right_padding() const
    {
	return m_right_padding;
    }

    void PaddingBox::set_top_padding(std::int32_t padding)
    {
	m_top_padding = padding;
    }

    void PaddingBox::set_bottom_padding(std::int32_t padding)
    {
	m_bottom_padding = padding;
    }

    void PaddingBox::set_left_padding(std::int32_t padding)
    {
	m_left_padding = padding;
    }

    void PaddingBox::set_right_padding(std::int32_t padding)
    {
	m_right_padding = padding;
    }
}
