#ifndef INCLUDE_TOYTK_DETAIL_BUFFER_HXX_G2XL4JYE
#define INCLUDE_TOYTK_DETAIL_BUFFER_HXX_G2XL4JYE

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>

namespace toytk::detail
{
    class ShmFile
    {
	std::optional<std::string> m_path;
	int m_fd;
	off_t m_len;

    public:

	ShmFile();
	ShmFile(std::string_view, int, mode_t);

	ShmFile(ShmFile &&);
	ShmFile &operator=(ShmFile &&);

	~ShmFile();

	ShmFile(const ShmFile &) = delete;
	ShmFile &operator=(const ShmFile &) = delete;

	const std::optional<std::string> &get_path() const;
	int get_fd() const;
	off_t get_len() const;

	operator bool() const;

	void open(std::string_view, int, mode_t);
	void close();
	void unlink();
	void truncate(off_t);
    };

    class MMapping
    {
	int m_fd;
	void *m_addr;
	size_t m_len;

    public:

	MMapping();
	MMapping(int, size_t, int, int, off_t);

	MMapping(MMapping &&);
	MMapping &operator=(MMapping &&);

	~MMapping();

	MMapping(const MMapping &) = delete;
	MMapping &operator=(MMapping &) = delete;

	int get_fd() const;
	size_t get_len() const;
	void *get_addr() const;

	operator bool() const;

	void mmap(int, size_t, int, int, off_t);
	void munmap();
    };

    struct WlShmPoolDelete
    {
	void operator()(wl_shm_pool *p) const;
    };

    using WlShmPoolPtr = std::unique_ptr<wl_shm_pool, WlShmPoolDelete>;

    struct WlBufferDelete
    {
	void operator()(wl_buffer *p) const;
    };

    using WlBufferPtr = std::unique_ptr<wl_buffer, WlBufferDelete>;

    class Buffer
    {
	wl_shm *m_shm;
	std::string m_path_prefix;
	ShmFile m_file;
	MMapping m_mapping;
	WlShmPoolPtr m_shm_pool;
	WlBufferPtr m_buffer;
	std::int32_t m_width = -1;
	std::int32_t m_height = -1;

    public:

	Buffer(wl_shm *, std::string_view);

	Buffer(const Buffer &) = delete;
	Buffer(Buffer &&) = delete;
	Buffer &operator=(const Buffer &) = delete;
	Buffer &operator=(Buffer &&) = delete;

	wl_buffer *get_buffer() const;
	int get_fd() const;
	void *get_addr();

	void resize(std::int32_t, std::int32_t);

	void destroy();
    };
}

#endif
