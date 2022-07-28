#pragma once
#include <iostream>
#include <limits>
#include <utility>

template<class T>
class MyAllocator
{
public:
	using value_type = T;
	using pointer = T*;
	using void_pointer = void*;
	using const_void_pointer = const void*;
	using size_type = size_t;
	using different_type = std::ptrdiff_t;

	MyAllocator() = default;
	~MyAllocator() = default;

	pointer allocate(size_type numObjs)
	{
		allocCount += numObjs;
		std::cout << "MyAllocator::allocate, 内存分配:" << numObjs << std::endl;
		return static_cast<pointer>(operator new(sizeof(T) * numObjs));
	}

	pointer allocate(size_type numObjs, const_void_pointer hit)
	{
		return allocate(numObjs);
	}

	void deallocate(pointer p, size_type numObjs)
	{
		std::cout << "MyAllocator::deallocate, 内存释放:" << numObjs << std::endl;
		allocCount -= numObjs;
		operator delete(p);
	}

	size_type max_size() const
	{
		return std::numeric_limits<size_type>::max();
	}

	size_type get_allocations() const
	{
		return allocCount;
	}

	// template<class U,class... Args>
	// void construct(U* p, Args &&...)
	// {
	// 	new(p) U(std::forward<Args>(args)...);
	// }
	//
	// template<class U, class... Args>
	// void destroy(U* p)
	// {
	// 	p->~U();
	// }

	template<class U>
	struct rebind
	{
		using other = MyAllocator<U>;
	};

private:
	size_type allocCount;
};

