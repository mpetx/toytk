#ifndef INCLUDE_TOYTK_WIDGET_FRAME_HXX_V2ESBN6H
#define INCLUDE_TOYTK_WIDGET_FRAME_HXX_V2ESBN6H

#include <toytk/core.hxx>
#include <toytk/font.hxx>

#include <string>
#include <string_view>

namespace toytk
{
    class Frame : public Widget
    {
	std::optional<std::reference_wrapper<Widget>> m_content;
	std::string m_title;
	Font m_font;

	static constexpr std::int32_t border_width = 8;
	static constexpr std::int32_t title_font_size = 16;
	static constexpr std::int32_t title_bar_width = border_width * 2 + title_font_size;

	static constexpr double frame_color_level = .867;
	static constexpr double border_color_level = .267;

	Position get_content_position() const;
	Dimension get_content_dimension() const;

	bool is_title_bar(double, double);
	bool is_top_resize_area(double, double);
	bool is_bottom_resize_area(double, double);
	bool is_left_resize_area(double, double);
	bool is_right_resize_area(double, double);

	static Bubbling frame_event_handler(Widget &, Window &, const Event &);

    public:

	Frame(bool);

	void layout() override;
	void paint(cairo_t *) const override;
	Dimension get_preferred_dimension() const override;
	void for_each_child(void (*)(Widget &, void *), void *) override;

	std::optional<std::reference_wrapper<Widget>> get_content() const;
	void set_content(Widget &);
	void reset_content();

	const Font &get_font() const;
	void set_font(const Font &);

	const std::string &get_title() const;
	void set_title(std::string_view);
    };
}

#endif
