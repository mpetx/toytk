#ifndef INCLUDE_TOYTK_COMMON_HXX_B6QPES5K
#define INCLUDE_TOYTK_COMMON_HXX_B6QPES5K

#include <cstdint>
#include <functional>
#include <memory>
#include <memory_resource>

namespace toytk
{
    struct Position
    {
	std::int32_t x, y;
    };

    struct Dimension
    {
	std::int32_t width, height;
    };

    template <typename T>
    class PmrDelete
    {
	std::pmr::memory_resource *m_memory_resource;

    public:

	PmrDelete();
	explicit PmrDelete(std::pmr::memory_resource *);
	PmrDelete(const PmrDelete &);
	PmrDelete(PmrDelete &&);
	PmrDelete &operator=(const PmrDelete &);
	PmrDelete &operator=(PmrDelete &&);

	void operator()(T *) const;
    };

    template <typename T>
    using PmrPtr = std::unique_ptr<T, PmrDelete<T>>;
}

namespace toytk
{
    template <typename T>
    PmrDelete<T>::PmrDelete()
	: m_memory_resource { std::pmr::get_default_resource() }
    {
    }

    template <typename T>
    PmrDelete<T>::PmrDelete(std::pmr::memory_resource *mr)
	: m_memory_resource { mr }
    {
    }

    template <typename T>
    PmrDelete<T>::PmrDelete(const PmrDelete &del)
	: m_memory_resource { del.m_memory_resource }
    {
    }

    template <typename T>
    PmrDelete<T>::PmrDelete(PmrDelete &&del)
	: m_memory_resource { del.m_memory_resource }
    {
	del.m_memory_resource = std::pmr::get_default_resource();
    }

    template <typename T>
    PmrDelete<T> &PmrDelete<T>::operator=(const PmrDelete &del)
    {
	m_memory_resource = del.m_memory_resource;

	return *this;
    }

    template <typename T>
    PmrDelete<T> &PmrDelete<T>::operator=(PmrDelete &&del)
    {
	if (this == &del)
	{
	    return *this;
	}

	m_memory_resource = del.m_memory_resource;

	del.m_memory_resource = std::pmr::get_default_resource();

	return *this;
    }

    template <typename T>
    void PmrDelete<T>::operator()(T *p) const
    {
	if (p)
	{
	    std::pmr::polymorphic_allocator<T> alloc { m_memory_resource };
	    alloc.delete_object(p);
	}
    }
}

#endif
