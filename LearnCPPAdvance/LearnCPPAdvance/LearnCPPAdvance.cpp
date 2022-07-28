// LearnCPPAdvance.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cinttypes>
#include <iostream>
#include <stdlib.h>
// #include <unistd.h>
#include <set>
#include <thread>
#include <vector>
#include "MyAllocator.h"

class MaObjectDisplay1;
using namespace std;

//int main()
//{
//    std::cout << "Hello World!\n";
//
//    std::vector<int, MyAllocator<int>> v(0);
//    std::vector<int, std::allocator<int>> test(0);
//
//    for (size_t i = 0; i < 30; ++i)
//    {
//        // _sleep(1);
//        // std::this_thread::sleep_for(1);
//        v.push_back(i);
//
//        std::cout << "当前容器占用: " << v.get_allocator().get_allocations() << std::endl; 
//    }
//
//}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
