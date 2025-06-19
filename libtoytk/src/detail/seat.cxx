
#include <toytk/detail/seat.hxx>

#include <sys/mman.h>
#include <unistd.h>

namespace toytk::detail
{
    void WlSeatDelete::operator()(wl_seat *p) const
    {
	if (p)
	{
	    wl_seat_release(p);
	}
    }

    void WlPointerDelete::operator()(wl_pointer *p) const
    {
	if (p)
	{
	    wl_pointer_release(p);
	}
    }

    void WlKeyboardDelete::operator()(wl_keyboard *p) const
    {
	if (p)
	{
	    wl_keyboard_release(p);
	}
    }

    void XkbKeymapDelete::operator()(xkb_keymap *p) const
    {
	if (p)
	{
	    xkb_keymap_unref(p);
	}
    }

    void XkbStateDelete::operator()(xkb_state *p) const
    {
	if (p)
	{
	    xkb_state_unref(p);
	}
    }

    Pointer::Pointer(Seat &seat)
	: m_seat { seat }
    {
	m_pointer.reset(wl_seat_get_pointer(m_seat.get_seat()));
	wl_pointer_add_listener(m_pointer.get(), &pointer_listener, this);
    }

    wl_pointer *Pointer::get_pointer() const
    {
	return m_pointer.get();
    }

    Seat &Pointer::get_seat() const
    {
	return m_seat;
    }

    void Pointer::handle_pointer_enter(void *data, wl_pointer *, std::uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
    {
	Pointer &pointer = *reinterpret_cast<Pointer *>(data);
	Seat &seat = pointer.get_seat();
	auto keyboard = seat.get_keyboard();
	Display &disp = seat.get_display();

	pointer.m_surface = surface;
	pointer.m_x = wl_fixed_to_double(x);
	pointer.m_y = wl_fixed_to_double(y);

	PointerEnterLowEvent event {
	    surface,
	    seat.get_seat(),
	    serial,
	    pointer.m_x, pointer.m_y,
	    keyboard.has_value() ? keyboard->get().get_state() : nullptr
	};

	disp.handle_low_event(event);
    }

    void Pointer::handle_pointer_leave(void *data, wl_pointer *, std::uint32_t serial, wl_surface *surface)
    {
	Pointer &pointer = *reinterpret_cast<Pointer *>(data);
	Seat &seat = pointer.get_seat();
	auto keyboard = seat.get_keyboard();
	Display &disp = seat.get_display();

	pointer.m_surface = nullptr;
	pointer.m_scroll = false;

	PointerLeaveLowEvent event {
	    surface,
	    seat.get_seat(),
	    serial,
	    keyboard.has_value() ? keyboard->get().get_state() : nullptr
	};

	disp.handle_low_event(event);
    }

    void Pointer::handle_pointer_motion(void *data, wl_pointer *, std::uint32_t time, wl_fixed_t x, wl_fixed_t y)
    {
	Pointer &pointer = *reinterpret_cast<Pointer *>(data);
	Seat &seat = pointer.get_seat();
	auto keyboard = seat.get_keyboard();
	Display &disp = seat.get_display();

	pointer.m_x = wl_fixed_to_double(x);
	pointer.m_y = wl_fixed_to_double(y);

	PointerMotionLowEvent event {
	    pointer.m_surface,
	    seat.get_seat(),
	    pointer.m_x, pointer.m_y,
	    keyboard.has_value() ? keyboard->get().get_state() : nullptr
	};

	disp.handle_low_event(event);
    }

    void Pointer::handle_pointer_button(void *data, wl_pointer *, std::uint32_t serial, std::uint32_t time, std::uint32_t button, std::uint32_t state)
    {
	Pointer &pointer = *reinterpret_cast<Pointer *>(data);
	Seat &seat = pointer.get_seat();
	auto keyboard = seat.get_keyboard();
	Display &disp = seat.get_display();

	PointerButtonLowEvent event {
	    pointer.m_surface,
	    seat.get_seat(),
	    serial,
	    button,
	    state,
	    pointer.m_x, pointer.m_y,
	    keyboard.has_value() ? keyboard->get().get_state() : nullptr
	};

	disp.handle_low_event(event);
    }

    void Pointer::handle_pointer_axis(void *data, wl_pointer *, std::uint32_t time, std::uint32_t axis, wl_fixed_t value)
    {
	Pointer &pointer = *reinterpret_cast<Pointer *>(data);

	if (!pointer.m_scroll)
	{
	    pointer.m_horizontal_scroll = 0;
	    pointer.m_vertical_scroll = 0;
	    pointer.m_scroll = true;
	}

	if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
	{
	    pointer.m_horizontal_scroll = wl_fixed_to_double(value);
	}
	else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
	{
	    pointer.m_vertical_scroll = wl_fixed_to_double(value);
	}
    }

    void Pointer::handle_pointer_frame(void *data, wl_pointer *)
    {
	Pointer &pointer = *reinterpret_cast<Pointer *>(data);
	Seat &seat = pointer.get_seat();
	auto keyboard = seat.get_keyboard();
	Display &disp = seat.get_display();

	if (pointer.m_scroll)
	{
	    pointer.m_scroll = false;

	    PointerAxisLowEvent event {
		pointer.m_surface,
		seat.get_seat(),
		pointer.m_horizontal_scroll,
		pointer.m_vertical_scroll,
		pointer.m_x, pointer.m_y,
		keyboard.has_value() ? keyboard->get().get_state() : nullptr
	    };

	    disp.handle_low_event(event);
	}
    }

    const wl_pointer_listener Pointer::pointer_listener {
	handle_pointer_enter,
	handle_pointer_leave,
	handle_pointer_motion,
	handle_pointer_button,
	handle_pointer_axis,
	handle_pointer_frame,
	[](void *, wl_pointer *, std::uint32_t) { },
	[](void *, wl_pointer *, std::uint32_t, std::uint32_t) { },
	[](void *, wl_pointer *, std::uint32_t, std::int32_t) { }
    };

    Keyboard::Keyboard(Seat &seat)
	: m_seat { seat }
    {
	m_keyboard.reset(wl_seat_get_keyboard(seat.get_seat()));
	wl_keyboard_add_listener(m_keyboard.get(), &keyboard_listener, this);
    }

    wl_keyboard *Keyboard::get_keyboard() const
    {
	return m_keyboard.get();
    }

    Seat &Keyboard::get_seat() const
    {
	return m_seat;
    }

    xkb_keymap *Keyboard::get_keymap() const
    {
	return m_keymap.get();
    }

    xkb_state *Keyboard::get_state() const
    {
	return m_state.get();
    }

    const Keyboard::RepeatInfo &Keyboard::get_repeat_info() const
    {
	return m_repeat_info;
    }

    void Keyboard::handle_keyboard_keymap(void *data, wl_keyboard *, std::uint32_t format, int fd, std::uint32_t size)
    {
	Keyboard &keyboard = *reinterpret_cast<Keyboard *>(data);
	Display &disp = keyboard.get_seat().get_display();

	keyboard.m_state.reset();
	keyboard.m_keymap.reset();

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
	{
	    close(fd);
	    return;
	}

	void *addr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (addr == MAP_FAILED)
	{
	    return;
	}

	char *str = reinterpret_cast<char *>(addr);
	keyboard.m_keymap.reset(xkb_map_new_from_string(disp.get_xkb_context(), str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS));
	munmap(addr, size);
	if (!keyboard.m_keymap)
	{
	    return;
	}

	keyboard.m_state.reset(xkb_state_new(keyboard.m_keymap.get()));
	if (!keyboard.m_state)
	{
	    keyboard.m_keymap.reset();
	}
    }

    void Keyboard::handle_keyboard_enter(void *data, wl_keyboard *, std::uint32_t serial, wl_surface *surface, wl_array *keys)
    {
	Keyboard &keyboard = *reinterpret_cast<Keyboard *>(data);
	Seat &seat = keyboard.get_seat();
	Display &disp = seat.get_display();

	keyboard.m_surface = surface;

	KeyboardEnterLowEvent event {
	    surface,
	    seat.get_seat(),
	    serial,
	    keyboard.get_state()
	};

	disp.handle_low_event(event);
    }

    void Keyboard::handle_keyboard_leave(void *data, wl_keyboard *, std::uint32_t serial, wl_surface *surface)
    {
	Keyboard &keyboard = *reinterpret_cast<Keyboard *>(data);
	Seat &seat = keyboard.get_seat();
	Display &disp = seat.get_display();

	keyboard.m_surface = nullptr;

	KeyboardLeaveLowEvent event {
	    surface,
	    seat.get_seat(),
	    serial,
	    keyboard.get_state()
	};

	disp.handle_low_event(event);
    }

    void Keyboard::handle_keyboard_key(void *data, wl_keyboard *, std::uint32_t serial, std::uint32_t time, std::uint32_t key, std::uint32_t state)
    {
	Keyboard &keyboard = *reinterpret_cast<Keyboard *>(data);
	Seat &seat = keyboard.get_seat();
	Display &disp = seat.get_display();

	if (!keyboard.m_state)
	{
	    return;
	}

	xkb_keysym_t sym = xkb_state_key_get_one_sym(keyboard.m_state.get(), key + 8);

	KeyboardKeyLowEvent event {
	    keyboard.m_surface,
	    seat.get_seat(),
	    serial,
	    sym,
	    state,
	    keyboard.get_state()
	};

	disp.handle_low_event(event);
    }

    void Keyboard::handle_keyboard_modifiers(void *data, wl_keyboard *, std::uint32_t serial, std::uint32_t mod_depressed, std::uint32_t mod_latched, std::uint32_t mod_locked, std::uint32_t group)
    {
	Keyboard &keyboard = *reinterpret_cast<Keyboard *>(data);

	if (!keyboard.m_state)
	{
	    return;
	}

	xkb_state_update_mask(keyboard.m_state.get(), mod_depressed, mod_latched, mod_locked, 0, 0, group);
    }

    void Keyboard::handle_keyboard_repeat_info(void *data, wl_keyboard *, std::int32_t rate, std::int32_t delay)
    {
	Keyboard &keyboard = *reinterpret_cast<Keyboard *>(data);

	keyboard.m_repeat_info.rate = rate;
	keyboard.m_repeat_info.delay = delay;
    }

    const wl_keyboard_listener Keyboard::keyboard_listener {
	handle_keyboard_keymap,
	handle_keyboard_enter,
	handle_keyboard_leave,
	handle_keyboard_key,
	handle_keyboard_modifiers,
	handle_keyboard_repeat_info
    };

    Seat::Seat(Display &disp, wl_seat *seat)
	: m_display { disp }
    {
	m_seat.reset(seat);
	wl_seat_add_listener(m_seat.get(), &seat_listener, this);
    }

    wl_seat *Seat::get_seat() const
    {
	return m_seat.get();
    }

    Display &Seat::get_display() const
    {
	return m_display;
    }

    const std::string &Seat::get_name() const
    {
	return m_name;
    }

    std::optional<std::reference_wrapper<Pointer>> Seat::get_pointer() const
    {
	if (m_pointer)
	{
	    return *m_pointer;
	}
	else
	{
	    return std::nullopt;
	}
    }

    std::optional<std::reference_wrapper<Keyboard>> Seat::get_keyboard() const
    {
	if (m_keyboard)
	{
	    return *m_keyboard;
	}
	else
	{
	    return std::nullopt;
	}
    }

    void Seat::handle_seat_capabilities(void *data, wl_seat *, std::uint32_t capabilities)
    {
	Seat &seat = *reinterpret_cast<Seat *>(data);

	bool has_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER) != 0;

	if (!seat.m_pointer && has_pointer)
	{
	    seat.m_pointer.reset(new Pointer { seat });
	}
	else if (seat.m_pointer && !has_pointer)
	{
	    seat.m_pointer.reset();
	}

	bool has_keyboard = (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) != 0;

	if (!seat.m_keyboard && has_keyboard)
	{
	    seat.m_keyboard.reset(new Keyboard { seat });
	}
	else if (seat.m_keyboard && !has_keyboard)
	{
	    seat.m_keyboard.reset();
	}
    }

    void Seat::handle_seat_name(void *data, wl_seat *, const char *name)
    {
	Seat &seat = *reinterpret_cast<Seat *>(data);
	seat.m_name = name;
    }

    const wl_seat_listener Seat::seat_listener {
	handle_seat_capabilities,
	handle_seat_name
    };
}
