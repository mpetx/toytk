#ifndef INCLUDE_TOYTK_EVENT_HXX_P9XFTYDM
#define INCLUDE_TOYTK_EVENT_HXX_P9XFTYDM

#include <xkbcommon/xkbcommon.h>
#include <linux/input-event-codes.h>
#include <wayland-client.h>

#include <cstdint>
#include <variant>

namespace toytk
{
    struct Serial
    {
	wl_seat *seat;
	std::uint32_t serial;
    };

    struct PointerEnterEvent
    {
	double x, y;
	xkb_state *state;
    };

    struct PointerLeaveEvent
    {
	xkb_state *state;
    };

    struct PointerMotionEvent
    {
	double x, y;
	xkb_state *state;
    };

    struct ButtonDownEvent
    {
	Serial serial;
	std::uint32_t button;
	double x, y;
	xkb_state *state;
    };

    struct ButtonUpEvent
    {
	Serial serial;
	std::uint32_t button;
	double x, y;
	xkb_state *state;
    };

    struct PointerAxisEvent
    {
	double horizontal_scroll;
	double vertical_scroll;
	double x, y;
	xkb_state *state;
    };

    struct FocusEvent
    {
	xkb_state *state;
    };

    struct BlurEvent
    {
	xkb_state *state;
    };

    struct KeyDownEvent
    {
	Serial serial;
	xkb_keysym_t key;
	xkb_state *state;
    };

    struct KeyUpEvent
    {
	Serial serial;
	xkb_keysym_t key;
	xkb_state *state;
    };

    using Event = std::variant<
	PointerEnterEvent,
	PointerLeaveEvent,
	PointerMotionEvent,
	ButtonDownEvent,
	ButtonUpEvent,
	PointerAxisEvent,
	FocusEvent,
	BlurEvent,
	KeyDownEvent,
	KeyUpEvent>;

    enum class Bubbling
    {
	bubble,
	stop
    };

    bool is_bubbling_event(const Event &);
}

#endif
 
