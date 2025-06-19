#ifndef INCLUDE_TOYTK_WIDGET_LABEL_HXX_J7QLTPZH
#define INCLUDE_TOYTK_WIDGET_LABEL_HXX_J7QLTPZH

#include <toytk/core.hxx>
#include <toytk/font.hxx>

#include <string>
#include <string_view>

namespace toytk
{
    class Label : public Widget
    {
	std::string m_text;
	Font m_font;

    public:

	Label();

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
