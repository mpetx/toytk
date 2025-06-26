
#include <toytk/widget/vertical-box.hxx>

#include <algorithm>

namespace toytk
{
    VerticalBox::VerticalBox()
    {
    }

    void VerticalBox::layout()
    {
	auto pos = get_position();
	auto dim = get_dimension();
	std::int32_t y_acc = pos.y;

	for (auto &child : m_children)
	{
	    auto cdim = child->get_preferred_dimension();
	    child->set_position(Position { pos.x, y_acc });
	    child->set_dimension(Dimension { dim.width, cdim.height });
	    y_acc += cdim.height;
	}
    }

    void VerticalBox::paint(cairo_t *) const
    {
    }

    Dimension VerticalBox::get_preferred_dimension() const
    {
	Dimension dim { 0, 0 };

	for (auto &child : m_children)
	{
	    auto cdim = child->get_preferred_dimension();
	    dim.width = std::max(cdim.width, dim.width);
	    dim.height += cdim.height;
	}

	return dim;
    }

    void VerticalBox::for_each_child(void (*func)(Widget &, void *), void *data)
    {
	std::ranges::for_each(m_children, [func, data](auto &widget) {
	    func(*widget, data);
	});
    }

    void VerticalBox::add_child(PmrPtr<Widget> &&child)
    {
	m_children.push_back(std::move(child));
	own_child(m_children[m_children.size() - 1]);
    }

    void VerticalBox::delete_child(Widget &child)
    {
	auto i = std::ranges::find_if(m_children, [&child](auto &p) {
	    return p.get() == &child;
	});

	if (i == m_children.end())
	{
	    return;
	}

	unown_child(*i);
	m_children.erase(i);
    }
}
