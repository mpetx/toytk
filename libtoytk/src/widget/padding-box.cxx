
#include <toytk/widget/padding-box.hxx>

#include <algorithm>

namespace toytk
{
    PaddingBox::PaddingBox()
    {
    }

    void PaddingBox::layout()
    {
	if (!m_content)
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

	m_content->set_position(cpos);

	Dimension cdim {
	    std::max(dim.width - m_left_padding - m_right_padding, 1),
	    std::max(dim.height - m_top_padding - m_bottom_padding, 1)
	};

	m_content->set_dimension(cdim);
    }

    void PaddingBox::paint(cairo_t *cr) const
    {
    }

    Dimension PaddingBox::get_preferred_dimension() const
    {
	if (m_content)
	{
	    auto dim = m_content->get_preferred_dimension();
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
	if (m_content)
	{
	    func(*m_content, data);
	}
    }

    std::optional<std::reference_wrapper<Widget>> PaddingBox::get_content() const
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

    PmrPtr<Widget> PaddingBox::set_content(PmrPtr<Widget> &&widget)
    {
	PmrPtr<Widget> old_content = reset_content();

	m_content = std::move(widget);
	own_child(m_content);

	return old_content;
    }

    PmrPtr<Widget> PaddingBox::reset_content()
    {
	if (m_content)
	{
	    unown_child(m_content);
	}

	return std::move(m_content);
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
