#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "MyAllocator.h"
using namespace std;

class MaObjectDisplay1
{
private:
	std::string DisplayString;
public:
    MaObjectDisplay1(const char* str)
        :DisplayString(str)
    {
        DisplayString += '\n';
    }

    void operator() (const int& inObj)
    {
        printf("inobj %d\n", inObj);
    }

    bool operator < (const MaObjectDisplay1& in) const
    {
        return false;
    }
};

//this alloc class is just for the stl set<> allocator  
template <typename T>
class MyAllc : public allocator<T>
{
public:
    typedef size_t   size_type;
    typedef typename allocator<T>::pointer              pointer;
    typedef typename allocator<T>::value_type           value_type;
    typedef typename allocator<T>::const_pointer        const_pointer;
    typedef typename allocator<T>::reference            reference;
    typedef typename allocator<T>::const_reference      const_reference;

    pointer allocate(size_type _Count, const void* _Hint = NULL)
    {
        void* rtn = NULL;
        //EthCFMMgntRbTreeMem::GetMemInstance()->malloc(_Count, rtn);  
        return (pointer)rtn;
    }

    void deallocate(pointer _Ptr, size_type _Count)
    {
        //EthCFMMgntRbTreeMem::GetMemInstance()->free(_Ptr);  
    }

    template<class _Other>
    struct rebind
    {   // convert this type to allocator<_Other>  
        typedef MyAllc<_Other> other;
    };

    MyAllc() throw()
    {}

    MyAllc(const MyAllc& __a) throw()
        : allocator<T>(__a)
    {}

    template<typename _Tp1>
    MyAllc(const MyAllc<_Tp1>&) throw()
    {}

    ~MyAllc() throw()
    {}
};


void Test2()
{
    std::cout << "Hello World!\n";
    
    std::vector<int, MyAllocator<int>> v(0);
    std::vector<int, std::allocator<int>> test(0);
    
    for (size_t i = 0; i < 30; ++i)
    {
        // _sleep(1);
        // std::this_thread::sleep_for(1);
        v.push_back(i);
    
        std::cout << "current container occupy: " << v.get_allocator().get_allocations() << std::endl; 
    }
}

void Test1()
{
    set<MaObjectDisplay1, std::less<MaObjectDisplay1 >, MyAllc<MaObjectDisplay1> > myset;

    vector<int, MyAllc<int>> myVector;
    myVector.push_back(1);

    MaObjectDisplay1 a("asdf");
    myset.insert(a);
    Test2();
}

int main()
{
    Test2();
}