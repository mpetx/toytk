#ifndef INCLUDE_TOYTK_DETAIL_OUTPUT_HXX_G67LW4XV
#define INCLUDE_TOYTK_DETAIL_OUTPUT_HXX_G67LW4XV

#include <wayland-client.h>

#include <cstdint>
#include <memory>

namespace toytk::detail
{
    struct WlOutputDelete
    {
	void operator()(wl_output *) const;
    };

    using WlOutputPtr = std::unique_ptr<wl_output, WlOutputDelete>;

    class Output
    {
    public:

	struct Geometry
	{
	    std::int32_t x, y;
	    std::int32_t physical_width, physical_height;
	    std::int32_t subpixel;
	    std::string make;
	    std::string model;
	    std::int32_t transform;
	};

	struct Mode
	{
	    std::int32_t width, height;
	    std::int32_t refresh;
	};

    private:

	WlOutputPtr m_output;
	Geometry m_geometry;
	Mode m_current_mode;
	std::int32_t m_scale = 1;

	static void handle_output_geometry(void *, wl_output *, std::int32_t, std::int32_t, std::int32_t, std::int32_t, std::int32_t, const char *, const char *, std::int32_t);
	static void handle_output_mode(void *, wl_output *, std::uint32_t, std::int32_t, std::int32_t, std::int32_t);
	static void handle_output_done(void *, wl_output *);
	static void handle_output_scale(void *, wl_output *, std::int32_t);
	static const wl_output_listener output_listener;

    public:

	explicit Output(wl_output *);

	Output(const Output &) = delete;
	Output(Output &&) = delete;
	Output &operator=(const Output &) = delete;
	Output &operator=(Output &&) = delete;

	wl_output *get_output() const;
	const Geometry &get_geometry() const;
	const Mode &get_current_mode() const;
	std::int32_t get_scale() const;
    };
}

#endif
