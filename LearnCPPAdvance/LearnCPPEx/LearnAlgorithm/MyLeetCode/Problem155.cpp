
#include <stack>
using namespace std;

namespace
{
    

class MinStack {

    stack<int> ValueStack;
    stack<int> MinValueStack;
    
public:
    MinStack() {
        MinValueStack.push(INT_MAX);
    }
    
    void push(int val) {

        ValueStack.push(val);
        MinValueStack.push(min(val, MinValueStack.top()));
    }
    
    void pop() {
        ValueStack.pop();
        MinValueStack.pop();
    }
    
    int top() {
        return  ValueStack.top();
    }
    
    int getMin() {
        return  MinValueStack.top();
    }
};

/**
 * Your MinStack object will be instantiated and called as such:
 * MinStack* obj = new MinStack();
 * obj->push(val);
 * obj->pop();
 * int param_3 = obj->top();
 * int param_4 = obj->getMin();
 */
}