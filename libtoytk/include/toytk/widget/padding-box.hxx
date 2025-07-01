#ifndef INCLUDE_TOYTK_WIDGET_PADDING_BOX_HXX_E297ATNJ
#define INCLUDE_TOYTK_WIDGET_PADDING_BOX_HXX_E297ATNJ

#include <toytk/core.hxx>

namespace toytk
{
    class PaddingBox : public Widget
    {
	PmrPtr<Widget> m_content;
	std::int32_t m_top_padding = 0;
	std::int32_t m_bottom_padding = 0;
	std::int32_t m_left_padding = 0;
	std::int32_t m_right_padding = 0;

    public:

	PaddingBox();

	void layout() override;
	void paint(cairo_t *) const override;
	Dimension get_preferred_dimension() const override;
	void for_each_child(void (*)(Widget &, void *), void *) override;

	std::optional<std::reference_wrapper<Widget>> get_content() const;
	PmrPtr<Widget> set_content(PmrPtr<Widget> &&);
	PmrPtr<Widget> reset_content();

	std::int32_t get_top_padding() const;
	std::int32_t get_bottom_padding() const;
	std::int32_t get_left_padding() const;
	std::int32_t get_right_padding() const;

	void set_top_padding(std::int32_t);
	void set_bottom_padding(std::int32_t);
	void set_left_padding(std::int32_t);
	void set_right_padding(std::int32_t);
    };
}

#endif
