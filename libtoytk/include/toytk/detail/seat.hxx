#ifndef INCLUDE_TOYTK_DETAIL_SEAT_HXX_F56SL8AP
#define INCLUDE_TOYTK_DETAIL_SEAT_HXX_F56SL8AP

#include <toytk/detail/display.hxx>

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace toytk::detail
{
    struct WlSeatDelete
    {
	void operator()(wl_seat *) const;
    };

    using WlSeatPtr = std::unique_ptr<wl_seat, WlSeatDelete>;

    struct WlPointerDelete
    {
	void operator()(wl_pointer *) const;
    };

    using WlPointerPtr = std::unique_ptr<wl_pointer, WlPointerDelete>;

    struct WlKeyboardDelete
    {
	void operator()(wl_keyboard *) const;
    };

    using WlKeyboardPtr = std::unique_ptr<wl_keyboard, WlKeyboardDelete>;

    struct XkbKeymapDelete
    {
	void operator()(xkb_keymap *) const;
    };

    using XkbKeymapPtr = std::unique_ptr<xkb_keymap, XkbKeymapDelete>;

    struct XkbStateDelete
    {
	void operator()(xkb_state *) const;
    };

    using XkbStatePtr = std::unique_ptr<xkb_state, XkbStateDelete>;

    class Display;
    class Seat;

    class Pointer
    {
	WlPointerPtr m_pointer;
	Seat &m_seat;
	wl_surface *m_surface = nullptr;
	double m_x, m_y;
	bool m_scroll = false;
	double m_horizontal_scroll;
	double m_vertical_scroll;

	static void handle_pointer_enter(void *, wl_pointer *, std::uint32_t, wl_surface *, wl_fixed_t, wl_fixed_t);
	static void handle_pointer_leave(void *, wl_pointer *, std::uint32_t, wl_surface *);
	static void handle_pointer_motion(void *, wl_pointer *, std::uint32_t, wl_fixed_t, wl_fixed_t);
	static void handle_pointer_button(void *, wl_pointer *, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
	static void handle_pointer_axis(void *, wl_pointer *, std::uint32_t, std::uint32_t, wl_fixed_t);
	static void handle_pointer_frame(void *, wl_pointer *);
	static const wl_pointer_listener pointer_listener;

    public:

	explicit Pointer(Seat &);

	Pointer(const Pointer &) = delete;
	Pointer(Pointer &&) = delete;
	Pointer &operator=(const Pointer &) = delete;
	Pointer &operator=(Pointer &&) = delete;

	wl_pointer *get_pointer() const;
	Seat &get_seat() const;
    };

    class Keyboard
    {
    public:

	struct RepeatInfo
	{
	    std::int32_t rate;
	    std::int32_t delay;
	};

    private:

	WlKeyboardPtr m_keyboard;
	Seat &m_seat;
	XkbKeymapPtr m_keymap;
	XkbStatePtr m_state;
	wl_surface *m_surface = nullptr;
	RepeatInfo m_repeat_info { 0, 0 };

	static void handle_keyboard_keymap(void *, wl_keyboard *, std::uint32_t, int, std::uint32_t);
	static void handle_keyboard_enter(void *, wl_keyboard *, std::uint32_t, wl_surface *, wl_array *);
	static void handle_keyboard_leave(void *, wl_keyboard *, std::uint32_t, wl_surface *);
	static void handle_keyboard_key(void *, wl_keyboard *, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
	static void handle_keyboard_modifiers(void *, wl_keyboard *, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
	static void handle_keyboard_repeat_info(void *, wl_keyboard *, std::int32_t, std::int32_t);
	static const wl_keyboard_listener keyboard_listener;

    public:

	explicit Keyboard(Seat &);

	Keyboard(const Keyboard &) = delete;
	Keyboard(Keyboard &&) = delete;
	Keyboard &operator=(const Keyboard &) = delete;
	Keyboard &operator=(Keyboard &&) = delete;

	wl_keyboard *get_keyboard() const;
	Seat &get_seat() const;
	xkb_keymap *get_keymap() const;
	xkb_state *get_state() const;
	const RepeatInfo &get_repeat_info() const;
    };

    class Seat
    {
	WlSeatPtr m_seat;
	Display &m_display;
	std::string m_name;
	std::unique_ptr<Pointer> m_pointer;
	std::unique_ptr<Keyboard> m_keyboard;

	static void handle_seat_capabilities(void *, wl_seat *, std::uint32_t);
	static void handle_seat_name(void *, wl_seat *, const char *);
	static const wl_seat_listener seat_listener;

    public:

	Seat(Display &, wl_seat *);

	Seat(const Seat &) = delete;
	Seat(Seat &&) = delete;
	Seat &operator=(const Seat &) = delete;
	Seat &operator=(Seat &&) = delete;

	wl_seat *get_seat() const;
	Display &get_display() const;
	const std::string &get_name() const;
	std::optional<std::reference_wrapper<Pointer>> get_pointer() const;
	std::optional<std::reference_wrapper<Keyboard>> get_keyboard() const;
    };
}

#endif
