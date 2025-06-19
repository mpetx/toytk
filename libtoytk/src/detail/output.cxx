
#include <toytk/detail/output.hxx>

namespace toytk::detail
{
    void WlOutputDelete::operator()(wl_output *p) const
    {
	if (p)
	{
	    wl_output_release(p);
	}
    }

    Output::Output(wl_output *output)
    {
	m_output.reset(output);
	wl_output_add_listener(m_output.get(), &output_listener, this);
    }

    wl_output *Output::get_output() const
    {
	return m_output.get();
    }

    const Output::Geometry &Output::get_geometry() const
    {
	return m_geometry;
    }

    const Output::Mode &Output::get_current_mode() const
    {
	return m_current_mode;
    }

    std::int32_t Output::get_scale() const
    {
	return m_scale;
    }

    void Output::handle_output_geometry(void *data, wl_output *, std::int32_t x, std::int32_t y, std::int32_t physical_width, std::int32_t physical_height, std::int32_t subpixel, const char *make, const char *model, std::int32_t transform)
    {
	Output &output = *reinterpret_cast<Output *>(data);
	output.m_geometry.x = x;
	output.m_geometry.y = y;
	output.m_geometry.physical_width = physical_width;
	output.m_geometry.physical_height = physical_height;
	output.m_geometry.make = make;
	output.m_geometry.model = model;
	output.m_geometry.transform = transform;
    }

    void Output::handle_output_mode(void *data, wl_output *, std::uint32_t flags, std::int32_t width, std::int32_t height, std::int32_t refresh)
    {
	if ((flags & WL_OUTPUT_MODE_CURRENT) == 0)
	{
	    return;
	}

	Output &output = *reinterpret_cast<Output *>(data);
	output.m_current_mode.width = width;
	output.m_current_mode.height = height;
	output.m_current_mode.refresh = refresh;
    }
    
    void Output::handle_output_done(void *, wl_output *)
    {
    }

    void Output::handle_output_scale(void *data, wl_output *, std::int32_t scale)
    {
	Output &output = *reinterpret_cast<Output *>(data);
	output.m_scale = scale;
    }

    const wl_output_listener Output::output_listener {
	handle_output_geometry,
	handle_output_mode,
	handle_output_done,
	handle_output_scale
    };
}
