

#include <functional>

namespace 
{
    class FTestModule
    {
    public:
        std::function<void()> testFunc;

        void StartupModule()
        {
            void TestInit();

            testFunc = TestInit;
        }

        void CallTestFunc()
        {
            testFunc();
        }
    };
}




void TestLoadModule()
{
    FTestModule testModule;
    testModule.StartupModule();
    testModule.CallTestFunc();
}