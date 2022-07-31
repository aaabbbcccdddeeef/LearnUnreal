#pragma once
#include <iostream>

class TypeTraitsTest
{
public:
    
};

enum TestEnum
{
    TYPE_1,
    TYPE_2,
    TYPE_3
};

enum TestEnum2
{
	TYPE_4,
	TYPE_5,
};

enum TestEnum3
{
    Type_6,
    Type_7
};

class Foo
{
public:
    //TestEnum type = TYPE_1;
    using TestType = TestEnum;
};

class Bar
{
public:
    //TestEnum type = TYPE_2;
	using TestType = TestEnum2;
};

template<typename T>
struct type_traits
{
    //TestEnum type = T::type;
	using TestType = typename T::TestType;
};

template<>
struct type_traits<int>
{
    //TestEnum type = TestEnum::TYPE_1;
    using TestType = TestEnum;
};


template<>
struct type_traits<double>
{
    //TestEnum type = TestEnum::TYPE_3;
    using TestType = TestEnum3;
};

template<typename T>
void decodeTest(const T& data, const char* buf)
{
  /*  if (typeid(type_traits<T>::TestType) == typeid(typename TestEnum::TestType))
    {
        std::cout << "decode type1 " << std::endl;
    } else if (typeid(type_traits<T>::TestType) == typeid(typename TestEnum3::TestType))
    {
		std::cout << "decode type2 " << std::endl;
    }*/
}


void TestTypeTraits()
{
    const char* test = "test";
    decodeTest(Foo(), test);

}