
#include "TestAtomic.hpp"

void main()
{
    // TestAtomicFlag();
    // TestAtomicBool();
    // TestAtomicOperation();



    for (int i = 0; i < 1000; ++i)
    {
        Test1::TesMemoryOrderRelax();
    }
}
