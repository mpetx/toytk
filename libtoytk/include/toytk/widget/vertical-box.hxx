#ifndef INCLUDE_TOYTK_WIDGET_VERTICAL_BOX_HXX_R2SJQ965
#define INCLUDE_TOYTK_WIDGET_VERTICAL_BOX_HXX_R2SJQ965

#include <toytk/core.hxx>

namespace toytk
{
    class VerticalBox : public Widget
    {
	std::vector<PmrPtr<Widget>> m_children;

    public:

	VerticalBox();
	
	void layout() override;
	void paint(cairo_t *) const override;
	Dimension get_preferred_dimension() const override;
	void for_each_child(void (*)(Widget &, void *), void *) override;

	void add_child(PmrPtr<Widget> &&);
	PmrPtr<Widget> remove_child(Widget &);
    };
}

#endif
