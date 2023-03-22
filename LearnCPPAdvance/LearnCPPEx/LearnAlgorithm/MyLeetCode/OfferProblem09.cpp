
#include <stack>
using namespace std;

namespace
{
    

class CQueue {

private:
    stack<int> InStack;
    stack<int> OutStack;

    void  InToOut()
    {
        while (!InStack.empty())
        {
            OutStack.push(InStack.top());
            InStack.pop();
        }
    }
    
public:
    CQueue() {

    }
    
    void appendTail(int value) {
        InStack.push(value);
    }
    
    int deleteHead() {

        if (OutStack.empty())
        {
            if (InStack.empty())
            {
                return -1;
            }
            InToOut();
        }
        int res = OutStack.top();
        OutStack.pop();
        return res;
    }
};

/**
 * Your CQueue object will be instantiated and called as such:
 * CQueue* obj = new CQueue();
 * obj->appendTail(value);
 * int param_2 = obj->deleteHead();
 */
}