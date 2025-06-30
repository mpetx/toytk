
#include <toytk/core.hxx>

#include <cairo/cairo.h>

namespace toytk
{
    namespace detail
    {
	void WlSurfaceDelete::operator()(wl_surface *p) const
	{
	    if (p)
	    {
		wl_surface_destroy(p);
	    }
	}

	void WlShellSurfaceDelete::operator()(wl_shell_surface *p) const
	{
	    if (p)
	    {
		wl_shell_surface_destroy(p);
	    }
	}

	void CairoSurfaceDelete::operator()(cairo_surface_t *p) const
	{
	    if (p)
	    {
		cairo_surface_destroy(p);
	    }
	}

	void CairoDelete::operator()(cairo_t *p) const
	{
	    if (p)
	    {
		cairo_destroy(p);
	    }
	}
    }

    Window::Window(Application &app, std::string_view title)
	: m_application { app },
	  m_title { title },
	  m_buffers {
	      { detail::application_get_display(app).get_shm(), "toytk" },
	      { detail::application_get_display(app).get_shm(), "toytk" }
	  }
    {
	detail::Display &disp = detail::application_get_display(app);

	m_surface.reset(wl_compositor_create_surface(disp.get_compositor()));
	wl_surface_add_listener(m_surface.get(), &surface_listener, this);

	m_shell_surface.reset(wl_shell_get_shell_surface(disp.get_shell(), m_surface.get()));
	wl_shell_surface_add_listener(m_shell_surface.get(), &shell_surface_listener, this);

	wl_shell_surface_set_toplevel(m_shell_surface.get());
	wl_shell_surface_set_class(m_shell_surface.get(), app.get_class().c_str());
	wl_shell_surface_set_title(m_shell_surface.get(), m_title.c_str());
	wl_surface_commit(m_surface.get());
    }

    Application &Window::get_application() const
    {
	return m_application;
    }

    const std::string &Window::get_title() const
    {
	return m_title;
    }

    void Window::set_title(std::string_view title)
    {
	m_title = title;
	wl_shell_surface_set_title(m_shell_surface.get(), m_title.c_str());
    }

    wl_output *Window::get_output() const
    {
	return m_output;
    }

    bool Window::get_should_close() const
    {
	return m_should_close;
    }

    void Window::set_should_close()
    {
	m_should_close = true;
    }

    Dimension Window::get_minimal_dimension() const
    {
	return m_minimal_dimension;
    }

    void Window::set_minimal_dimension(const Dimension &dim)
    {
	m_minimal_dimension = dim;
    }

    bool Window::get_redraw_needed() const
    {
	return m_redraw_needed;
    }

    void Window::set_redraw_needed()
    {
	m_redraw_needed = true;
    }

    std::optional<std::reference_wrapper<Widget>> Window::get_root() const
    {
	if (m_root)
	{
	    return *m_root;
	}
	else
	{
	    return std::nullopt;
	}
    }

    void Window::set_root(PmrPtr<Widget> &&widget)
    {
	m_root = std::move(widget);
	detail::widget_prepare_for_owned(*m_root);

	m_redraw_needed = true;
	redraw();
    }

    void Window::redraw()
    {
	if (!m_root || !m_surface)
	{
	    m_redraw_needed = false;
	    return;
	}

	int index = get_free_buffer_index();

	if (index == -1)
	{
	    return;
	}

	detail::Buffer &buffer = m_buffers[index];

	const Dimension &dim = m_root->get_dimension();

	buffer.resize(dim.width, dim.height);

	if (!buffer.get_buffer())
	{
	    return;
	}

	auto data = reinterpret_cast<unsigned char *>(buffer.get_addr());

	detail::CairoSurfacePtr surface { cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, dim.width, dim.height, dim.width * 4) };
	detail::CairoPtr cr { cairo_create(surface.get()) };

	cairo_set_source_rgba(cr.get(), 0, 0, 0, 0);
	cairo_paint(cr.get());

	m_root->draw_recursively(cr.get());

	cr.reset();
	surface.reset();

	if (!wl_proxy_get_listener(reinterpret_cast<wl_proxy *>(buffer.get_buffer())))
	{
	    wl_buffer_add_listener(buffer.get_buffer(), &buffer_listener, this);
	}
	m_redraw_needed = false;
	m_buffers_busy[index] = true;
	wl_surface_attach(m_surface.get(), buffer.get_buffer(), 0, 0);
	wl_surface_damage(m_surface.get(), 0, 0, dim.width, dim.height);
	wl_surface_commit(m_surface.get());
    }

    void Window::handle_seat_delete(wl_seat *seat)
    {
	m_hover_stacks.erase(seat);
	m_foci.erase(seat);
	m_grab.erase(seat);
    }

    void Window::notify_widget_removal(Widget &widget)
    {
    }

    void Window::move(const Serial &serial)
    {
	wl_shell_surface_move(m_shell_surface.get(), serial.seat, serial.serial);
    }

    void Window::resize(const Serial &serial, unsigned int edges)
    {
	bool top = (edges & edges_top) != 0;
	bool bottom = (edges & edges_bottom) != 0;
	bool left = (edges & edges_left) != 0;
	bool right = (edges & edges_right) != 0;

	std::uint32_t wl_edges =
	    bottom && right ? WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT
	    : top && right ? WL_SHELL_SURFACE_RESIZE_TOP_RIGHT
	    : bottom && left ? WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT
	    : top && left ? WL_SHELL_SURFACE_RESIZE_TOP_LEFT
	    : left ? WL_SHELL_SURFACE_RESIZE_LEFT
	    : right ? WL_SHELL_SURFACE_RESIZE_RIGHT
	    : bottom ? WL_SHELL_SURFACE_RESIZE_BOTTOM
	    : top ? WL_SHELL_SURFACE_RESIZE_TOP
	    : WL_SHELL_SURFACE_RESIZE_NONE;

	wl_shell_surface_resize(m_shell_surface.get(), serial.seat, serial.serial, wl_edges);
    }

    int Window::get_free_buffer_index() const
    {
	if (!m_buffers_busy[0])
	{
	    return 0;
	}
	else if (!m_buffers_busy[1])
	{
	    return 1;
	}
	else
	{
	    return -1;
	}
    }

    void Window::handle_surface_enter(void *data, wl_surface *, wl_output *output)
    {
	Window &win = *reinterpret_cast<Window *>(data);
	win.m_output = output;
    }

    void Window::handle_surface_leave(void *data, wl_surface *, wl_output *)
    {
	Window &win = *reinterpret_cast<Window *>(data);
	win.m_output = nullptr;
    }

    const wl_surface_listener Window::surface_listener {
	handle_surface_enter,
	handle_surface_leave
    };

    void Window::handle_shell_surface_ping(void *data, wl_shell_surface *, std::uint32_t serial)
    {
	Window &win = *reinterpret_cast<Window *>(data);
	wl_shell_surface_pong(win.m_shell_surface.get(), serial);
    }

    void Window::handle_shell_surface_configure(void *data, wl_shell_surface *, std::uint32_t edge, std::int32_t width, std::int32_t height)
    {
	Window &win = *reinterpret_cast<Window *>(data);

	if (win.m_root)
	{
	    win.m_root->set_dimension(Dimension {
		    std::max(width, win.m_minimal_dimension.width),
		    std::max(height, win.m_minimal_dimension.height)
		});
	    win.set_redraw_needed();
	    win.redraw();
	}
    }

    const wl_shell_surface_listener Window::shell_surface_listener {
	handle_shell_surface_ping,
	handle_shell_surface_configure,
	[](void *, wl_shell_surface *) { }
    };

    void Window::handle_buffer_release(void *data, wl_buffer *buffer)
    {
	Window &win = *reinterpret_cast<Window *>(data);

	if (win.m_buffers[0].get_buffer() == buffer)
	{
	    win.m_buffers_busy[0] = false;
	}
	else if (win.m_buffers[1].get_buffer() == buffer)
	{
	    win.m_buffers_busy[1] = false;
	}

	if (win.m_redraw_needed)
	{
	    win.redraw();
	}
    }

    const wl_buffer_listener Window::buffer_listener {
	handle_buffer_release
    };
}
