#pragma once
#include <iostream>
#include <limits>
#include <utility>

template<class T>
class MyAllocator : public std::allocator<T>
{
public:
	using value_type = T;
	using pointer = T*;
	using void_pointer = void*;
	using const_void_pointer = const void*;
	using size_type = size_t;
	using different_type = std::ptrdiff_t;

	MyAllocator() throw()
	{
	}

	MyAllocator(const T& other) throw()
		: std::allocator<T>(other)
	{

	}

	~MyAllocator() throw()
	{

	}

	template<typename _Tp1>
	MyAllocator(const MyAllocator<_Tp1>&) throw()
	{}

	pointer allocate(size_type numObjs)
	{
		allocCount += numObjs;
		std::cout << "MyAllocator::allocate, memory allocate:" << numObjs << std::endl;
		return static_cast<pointer>(operator new(sizeof(T) * numObjs));
	}

	pointer allocate(size_type numObjs, const_void_pointer hit)
	{
		return allocate(numObjs);
	}

	void deallocate(pointer p, size_type numObjs)
	{
		std::cout << "MyAllocator::deallocate, release memory:" << numObjs << std::endl;
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

	// 这个的目的是将all
	template<class U>
	struct rebind
	{
		using other = MyAllocator<U>;
	};


private:
	size_type allocCount;
};


//template<typename T>
//MyAllocator<T>::size_type allocCount = 0;
