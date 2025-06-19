
#include <toytk/detail/display.hxx>

#include <utility>

namespace toytk::detail
{
    void WlDisplayDelete::operator()(wl_display *p) const
    {
	if (p)
	{
	    wl_display_disconnect(p);
	}
    }

    void WlRegistryDelete::operator()(wl_registry *p) const
    {
	if (p)
	{
	    wl_registry_destroy(p);
	}
    }

    void WlCompositorDelete::operator()(wl_compositor *p) const
    {
	if (p)
	{
	    wl_compositor_destroy(p);
	}
    }

    void WlShmDelete::operator()(wl_shm *p) const
    {
	if (p)
	{
	    wl_shm_destroy(p);
	}
    }

    void WlShellDelete::operator()(wl_shell *p) const
    {
	if (p)
	{
	    wl_shell_destroy(p);
	}
    }

    void XkbContextDelete::operator()(xkb_context *p) const
    {
	if (p)
	{
	    xkb_context_unref(p);
	}
    }

    Display::Display(wl_display *disp)
    {
	if (!disp)
	{
	    return;
	}

	m_display.reset(disp);
	m_registry.reset(wl_display_get_registry(m_display.get()));
	m_xkb_context.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
	wl_registry_add_listener(m_registry.get(), &registry_listener, this);
	wl_display_roundtrip(m_display.get());
	wl_display_roundtrip(m_display.get());
	m_error = wl_display_get_error(m_display.get()) != 0;
    }

    void Display::initialize_compositor(std::uint32_t name)
    {
	void *p = wl_registry_bind(m_registry.get(), name, &wl_compositor_interface, 4);
	m_compositor.reset(reinterpret_cast<wl_compositor *>(p));
    }

    void Display::initialize_shm(std::uint32_t name)
    {
	void *p = wl_registry_bind(m_registry.get(), name, &wl_shm_interface, 1);
	m_shm.reset(reinterpret_cast<wl_shm *>(p));
	wl_shm_add_listener(m_shm.get(), &shm_listener, this);
    }

    void Display::initialize_shell(std::uint32_t name)
    {
	void *p = wl_registry_bind(m_registry.get(), name, &wl_shell_interface, 1);
	m_shell.reset(reinterpret_cast<wl_shell *>(p));
    }

    void Display::initialize_output(std::uint32_t name)
    {
	void *p = wl_registry_bind(m_registry.get(), name, &wl_output_interface, 3);
	m_outputs.emplace(name, std::make_unique<Output>(reinterpret_cast<wl_output *>(p)));
    }

    void Display::finalize_output(std::uint32_t name)
    {
	m_outputs.erase(name);
    }

    void Display::initialize_seat(std::uint32_t name)
    {
	void *p = wl_registry_bind(m_registry.get(), name, &wl_seat_interface, 7);
	m_seats.emplace(name, std::make_unique<Seat>(*this, reinterpret_cast<wl_seat *>(p)));
    }

    void Display::finalize_seat(std::uint32_t name)
    {
	auto i = m_seats.find(name);

	if (i == m_seats.end())
	{
	    return;
	}

	handle_seat_delete(i->second->get_seat());

	m_seats.erase(i);
    }

    wl_display *Display::get_display() const
    {
	return m_display.get();
    }

    wl_registry *Display::get_registry() const
    {
	return m_registry.get();
    }

    wl_compositor *Display::get_compositor() const
    {
	return m_compositor.get();
    }

    wl_shm *Display::get_shm() const
    {
	return m_shm.get();
    }

    wl_shell *Display::get_shell() const
    {
	return m_shell.get();
    }

    const std::unordered_map<std::uint32_t, std::unique_ptr<Output>> &Display::get_outputs() const
    {
	return m_outputs;
    }

    const std::unordered_map<std::uint32_t, std::unique_ptr<Seat>> &Display::get_seats() const
    {
	return m_seats;
    }

    xkb_context *Display::get_xkb_context() const
    {
	return m_xkb_context.get();
    }

    void Display::handle_low_event(const LowEvent &event)
    {
	if (on_low_event_func)
	{
	    on_low_event_func(event, on_low_event_data);
	}
    }

    void Display::handle_seat_delete(wl_seat *p)
    {
	if (on_seat_delete_func)
	{
	    on_seat_delete_func(p, on_seat_delete_data);
	}
    }

    Display::operator bool() const
    {
	return m_display
	    && m_registry
	    && m_compositor
	    && m_shm
	    && m_shell
	    && m_xkb_context
	    && !m_error;
    }

    void Display::dispatch_low_events()
    {
	int result = wl_display_dispatch(m_display.get());

	if (result == -1)
	{
	    m_error = true;
	}
	else
	{
	    m_error = wl_display_get_error(m_display.get()) != 0;
	}
    }

    const std::unordered_set<std::uint32_t> &Display::get_shm_formats() const
    {
	return m_shm_formats;
    }

    void Display::handle_registry_global(void *data, wl_registry *, std::uint32_t name, const char *c_interface, std::uint32_t version)
    {
	Display &disp = *reinterpret_cast<Display *>(data);
	std::string_view interface { c_interface };

	if (interface == "wl_compositor")
	{
	    disp.initialize_compositor(name);
	}
	else if (interface == "wl_shm")
	{
	    disp.initialize_shm(name);
	}
	else if (interface == "wl_shell")
	{
	    disp.initialize_shell(name);
	}
	else if (interface == "wl_output")
	{
	    disp.initialize_output(name);
	}
	else if (interface == "wl_seat")
	{
	    disp.initialize_seat(name);
	}
    }

    void Display::handle_registry_global_remove(void *data, wl_registry *, std::uint32_t name)
    {
	Display &disp = *reinterpret_cast<Display *>(data);
	disp.finalize_output(name);
	disp.finalize_seat(name);
    }

    const wl_registry_listener Display::registry_listener {
	handle_registry_global,
	handle_registry_global_remove
    };

    void Display::handle_shm_format(void *data, wl_shm *, std::uint32_t format)
    {
	Display &disp = *reinterpret_cast<Display *>(data);
	disp.m_shm_formats.emplace(format);
    }

    const wl_shm_listener Display::shm_listener {
	handle_shm_format
    };
}
