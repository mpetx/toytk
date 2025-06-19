#ifndef INCLUDE_TOYTK_WIDGET_BUTTON_HXX_D39MXZRC
#define INCLUDE_TOYTK_WIDGET_BUTTON_HXX_D39MXZRC

#include <toytk/core.hxx>
#include <toytk/font.hxx>

#include <string>
#include <string_view>

namespace toytk
{
    class Button : public Widget
    {
	std::string m_text;
	Font m_font;

	static Bubbling button_event_handler(Widget &, Window &, const Event &);

	static constexpr double normal_color_level = .867;
	static constexpr double hover_color_level = .733;
	static constexpr double border_color_level = .267;

	static constexpr std::int32_t preferred_x_padding = 16;
	static constexpr std::int32_t preferred_y_padding = 8;

    public:

	Button();

	void layout() override;
	void paint(cairo_t *) const override;
	Dimension get_preferred_dimension() const override;
	void for_each_child(void (*)(Widget &, void *), void *) override;

	const std::string &get_text() const;
	void set_text(std::string_view);

	const Font &get_font() const;
	void set_font(const Font &);
    };
}

#endif
