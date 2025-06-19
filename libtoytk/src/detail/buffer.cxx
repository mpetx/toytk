
#include <toytk/detail/buffer.hxx>

#include <fcntl.h>

namespace toytk::detail
{
    ShmFile::ShmFile()
	: m_fd { -1 },
	  m_len { 0 }
    {
    }

    ShmFile::ShmFile(std::string_view path, int oflag, mode_t mode)
	: ShmFile { }
    {
	open(path, oflag, mode);
    }

    ShmFile::ShmFile(ShmFile &&f)
	: m_path { f.m_path },
	  m_fd { f.m_fd },
	  m_len { f.m_len }
    {
	f.m_path = std::nullopt;
	f.m_fd = -1;
	f.m_len = 0;
    }

    ShmFile &ShmFile::operator=(ShmFile &&f)
    {
	if (this == &f)
	{
	    return *this;
	}

	close();

	m_path = f.m_path;
	m_fd = f.m_fd;
	m_len = f.m_len;

	f.m_path = std::nullopt;
	f.m_fd = -1;
	f.m_len = 0;

	return *this;
    }

    ShmFile::~ShmFile()
    {
	close();
    }

    const std::optional<std::string> &ShmFile::get_path() const
    {
	return m_path;
    }

    int ShmFile::get_fd() const
    {
	return m_fd;
    }

    off_t ShmFile::get_len() const
    {
	return m_len;
    }

    ShmFile::operator bool() const
    {
	return m_fd != -1;
    }

    void ShmFile::open(std::string_view path_sv, int oflag, mode_t mode)
    {
	close();

	m_path = path_sv;
	m_fd = shm_open(m_path->c_str(), oflag, mode);
	m_len = 0;

	if (m_fd == -1)
	{
	    m_path = std::nullopt;
	}
    }

    void ShmFile::close()
    {
	if (m_path.has_value())
	{
	    shm_unlink(m_path->c_str());
	}

	if (m_fd != -1)
	{
	    ::close(m_fd);
	}

	m_path = std::nullopt;
	m_fd = -1;
	m_len = 0;
    }

    void ShmFile::unlink()
    {
	if (!m_path.has_value())
	{
	    return;
	}

	shm_unlink(m_path->c_str());
	m_path = std::nullopt;
    }

    void ShmFile::truncate(off_t len)
    {
	if (m_fd == -1)
	{
	    return;
	}

	if (ftruncate(m_fd, len) != -1)
	{
	    m_len = len;
	}
    }

    MMapping::MMapping()
	: m_fd { -1 },
	  m_addr { nullptr },
	  m_len { 0 }
    {
    }

    MMapping::MMapping(int fd, size_t len, int prot, int flags, off_t offset)
	: MMapping { }
    {
	mmap(fd, len, prot, flags, offset);
    }

    MMapping::MMapping(MMapping &&map)
	: m_fd { map.m_fd },
	  m_addr { map.m_addr },
	  m_len { map.m_len }
    {
	map.m_fd = -1;
	map.m_addr = nullptr;
	map.m_len = 0;
    }

    MMapping &MMapping::operator=(MMapping &&map)
    {
	if (this == &map)
	{
	    return *this;
	}

	munmap();

	m_fd = map.m_fd;
	m_addr = map.m_addr;
	m_len = map.m_len;

	map.m_fd = -1;
	map.m_addr = nullptr;
	map.m_len = 0;

	return *this;
    }

    MMapping::~MMapping()
    {
	munmap();
    }

    int MMapping::get_fd() const
    {
	return m_fd;
    }

    void *MMapping::get_addr() const
    {
	return m_addr;
    }

    size_t MMapping::get_len() const
    {
	return m_len;
    }

    MMapping::operator bool() const
    {
	return static_cast<bool>(m_addr);
    }

    void MMapping::mmap(int fd, size_t len, int prot, int flags, off_t offset)
    {
	munmap();

	void *addr = ::mmap(nullptr, len, prot, flags, fd, offset);

	if (addr == MAP_FAILED)
	{
	    return;
	}

	m_fd = fd;
	m_addr = addr;
	m_len = len;
    }

    void MMapping::munmap()
    {
	if (m_addr)
	{
	    ::munmap(m_addr, m_len);
	    m_fd = -1;
	    m_addr = nullptr;
	    m_len = 0;
	}
    }

    void WlShmPoolDelete::operator()(wl_shm_pool *p) const
    {
	if (p)
	{
	    wl_shm_pool_destroy(p);
	}
    }

    void WlBufferDelete::operator()(wl_buffer *p) const
    {
	if (p)
	{
	    wl_buffer_destroy(p);
	}
    }

    Buffer::Buffer(wl_shm *shm, std::string_view prefix)
	: m_shm { shm },
	  m_path_prefix { prefix }
    {
    }

    wl_buffer *Buffer::get_buffer() const
    {
	return m_buffer.get();
    }

    int Buffer::get_fd() const
    {
	return m_file.get_fd();
    }

    void *Buffer::get_addr()
    {
	if (m_file.get_len() == 0)
	{
	    return nullptr;
	}

	if (!m_mapping || m_mapping.get_len() < m_file.get_len())
	{
	    m_mapping.mmap(m_file.get_fd(), m_file.get_len(), PROT_READ | PROT_WRITE, MAP_SHARED, 0);
	}

	return m_mapping.get_addr();
    }

    enum class FileCreationStatus
    {
	existed,
	created,
	failed
    };

    static FileCreationStatus ensure_file(ShmFile &f, std::string_view prefix)
    {
	if (f)
	{
	    return FileCreationStatus::existed;
	}

	int try_count = 0;

	while (!f && try_count < 256)
	{
	    std::string path = std::format("/{}-{}", prefix, try_count);
	    f.open(path, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
	    ++try_count;
	}

	if (f)
	{
	    f.unlink();
	    return FileCreationStatus::created;
	}
	else
	{
	    return FileCreationStatus::failed;
	}
    }

    enum class FileResizeStatus
    {
	remained,
	resized,
	failed
    };

    static FileResizeStatus ensure_file_size(ShmFile &f, off_t len)
    {
	if (f.get_len() >= len)
	{
	    return FileResizeStatus::remained;
	}

	f.truncate(len);

	if (f.get_len() >= len)
	{
	    return FileResizeStatus::resized;
	}
	else
	{
	    return FileResizeStatus::failed;
	}
    }

    void Buffer::resize(std::int32_t width, std::int32_t height)
    {
	if (m_width == width && m_height == height)
	{
	    return;
	}

	m_buffer.reset();
	m_width = m_height = -1;

	auto creation = ensure_file(m_file, m_path_prefix);

	if (creation == FileCreationStatus::failed)
	{
	    return;
	}

	std::int32_t len = width * height * 4;

	auto resize = ensure_file_size(m_file, len);

	if (resize == FileResizeStatus::failed)
	{
	    return;
	}

	if (!m_shm_pool)
	{
	    m_shm_pool.reset(wl_shm_create_pool(m_shm, m_file.get_fd(), m_file.get_len()));
	}
	else if (resize == FileResizeStatus::resized)
	{
	    wl_shm_pool_resize(m_shm_pool.get(), m_file.get_len());
	}

	m_buffer.reset(wl_shm_pool_create_buffer(m_shm_pool.get(), 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888));
	m_width = width;
	m_height = height;
    }

    void Buffer::destroy()
    {
	m_width = m_height = -1;
	m_buffer.reset();
	m_shm_pool.reset();
	m_mapping.munmap();
	m_file.close();
    }
}
