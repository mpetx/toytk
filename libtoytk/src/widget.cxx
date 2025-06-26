
#include <toytk/core.hxx>

#include <algorithm>

namespace toytk
{
    namespace detail
    {
	void BasicWidgetHandlerVisitor::operator()(const PointerEnterEvent &) const
	{
	    ++widget.m_hover_count;
	}

	void BasicWidgetHandlerVisitor::operator()(const PointerLeaveEvent &) const
	{
	    --widget.m_hover_count;
	}

	void BasicWidgetHandlerVisitor::operator()(const ButtonDownEvent &) const
	{
	    ++widget.m_activation_count;
	}

	void BasicWidgetHandlerVisitor::operator()(const ButtonUpEvent &) const
	{
	    --widget.m_activation_count;
	}

	void BasicWidgetHandlerVisitor::operator()(const FocusEvent &) const
	{
	    ++widget.m_focus_count;
	}

	void BasicWidgetHandlerVisitor::operator()(const BlurEvent &) const
	{
	    --widget.m_focus_count;
	}
    }

    Widget::Widget()
	: m_position { 0, 0 },
	  m_dimension { 1, 1 }
    {
	m_event_handlers.reserve(3);

	add_event_handler([](Widget &widget, Window &, const Event &event) {
	    detail::BasicWidgetHandlerVisitor visitor { widget };
	    std::visit(visitor, event);
	    return Bubbling::bubble;
	});
    }

    Widget::~Widget()
    {
    }

    const Position &Widget::get_position() const
    {
	return m_position;
    }

    void Widget::set_position(const Position &pos)
    {
	m_position = pos;
    }

    const Dimension &Widget::get_dimension() const
    {
	return m_dimension;
    }

    std::optional<std::reference_wrapper<Widget>> Widget::get_parent() const
    {
	return m_parent;
    }

    void Widget::own_child(PmrPtr<Widget> &child)
    {
	child->m_hover_count = 0;
	child->m_activation_count = 0;
	child->m_focus_count = 0;
	child->m_parent = *this;
    }

    void Widget::unown_child(PmrPtr<Widget> &child)
    {
	child->m_parent = std::nullopt;
    }

    bool Widget::is_hovered() const
    {
	return m_hover_count != 0;
    }

    bool Widget::is_active() const
    {
	return m_activation_count != 0;
    }

    bool Widget::is_focused() const
    {
	return m_focus_count != 0;
    }

    void Widget::add_event_handler(EventHandler handler)
    {
	m_event_handlers.push_back(handler);
    }

    void Widget::remove_event_handler(EventHandler handler)
    {
	for (auto i = m_event_handlers.begin(); i != m_event_handlers.end(); ++i)
	{
	    if (*i == handler)
	    {
		m_event_handlers.erase(i);
		break;
	    }
	}
    }

    bool Widget::contains_point(double x, double y) const
    {
	return m_position.x <= x
	    && x <= m_position.x + m_dimension.width
	    && m_position.y <= y
	    && y <= m_position.y + m_dimension.height;
    }

    std::optional<std::reference_wrapper<Widget>> Widget::get_child_at(double x, double y)
    {
	struct Info
	{
	    double x, y;
	    std::optional<std::reference_wrapper<Widget>> result;
	} info;

	info.x = x;
	info.y = y;

	for_each_child([](Widget &child, void *data) {
	    Info &info = *reinterpret_cast<Info *>(data);

	    if (child.contains_point(info.x, info.y))
	    {
		info.result = child;
	    }
	}, &info);

	return info.result;
    }

    std::vector<std::reference_wrapper<Widget>> Widget::get_widget_stack_below(double x, double y)
    {
	std::vector<std::reference_wrapper<Widget>> result;

	std::optional<std::reference_wrapper<Widget>> widget = *this;

	while (widget.has_value())
	{
	    result.push_back(widget->get());
	    widget = widget->get().get_child_at(x, y);
	}

	return result;
    }

    std::vector<std::reference_wrapper<Widget>> Widget::get_widget_stack_above()
    {
	std::vector<std::reference_wrapper<Widget>> result;

	std::optional<std::reference_wrapper<Widget>> widget = *this;

	while (widget.has_value())
	{
	    result.push_back(widget->get());
	    widget = widget->get().get_parent();
	}

	std::ranges::reverse(result);

	return result;
    }

    void Widget::set_dimension(const Dimension &dim)
    {
	m_dimension = dim;
    }

    void Widget::draw_recursively(cairo_t *cr)
    {
	layout();
	paint(cr);

	for_each_child([](Widget &child, void *data) {
	    cairo_t *cr = reinterpret_cast<cairo_t *>(data);
	    child.draw_recursively(cr);
	}, cr);
    }

    void Widget::preferize_dimension()
    {
	set_dimension(get_preferred_dimension());
    }

    void Widget::handle_event(Window &win, const Event &event)
    {
	std::optional<std::reference_wrapper<Widget>> widget = *this;
	
	while (widget.has_value())
	{
	    for (auto handler : widget->get().m_event_handlers)
	    {
		Bubbling bubbling = handler(widget->get(), win, event);

		if (bubbling == Bubbling::stop)
		{
		    return;
		}
	    }

	    if (!is_bubbling_event(event))
	    {
		break;
	    }

	    widget = widget->get().get_parent();
	}
    }
}
