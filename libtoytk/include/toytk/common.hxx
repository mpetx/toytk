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
    class PolymorphicDelete
    {
	std::reference_wrapper<std::pmr::polymorphic_allocator<T>> m_allocator;

    public:

	explicit PolymorphicDelete(std::pmr::polymorphic_allocator<T> &);
	PolymorphicDelete(const PolymorphicDelete &);
	PolymorphicDelete &operator=(const PolymorphicDelete &);

	PolymorphicDelete() = delete;
	PolymorphicDelete(PolymorphicDelete &&) = delete;
	PolymorphicDelete &operator=(PolymorphicDelete &&) = delete;

	void operator()(T *) const;
    };

    template <typename T>
    using PolymorphicPtr = std::unique_ptr<T, PolymorphicDelete<T>>;
}

namespace toytk
{
    template <typename T>
    PolymorphicDelete<T>::PolymorphicDelete(std::pmr::polymorphic_allocator<T> &alloc)
	: m_allocator { alloc }
    {
    }

    template <typename T>
    PolymorphicDelete<T>::PolymorphicDelete(const PolymorphicDelete &del)
	: m_allocator { del.m_allocator }
    {
    }

    template <typename T>
    PolymorphicDelete<T> &PolymorphicDelete<T>::operator=(const PolymorphicDelete &del)
    {
	m_allocator = del.m_allocator;
    }

    template <typename T>
    void PolymorphicDelete<T>::operator()(T *p) const
    {
	if (p)
	{
	    m_allocator->get().delete_object(p);
	}
    }
}

#endif
