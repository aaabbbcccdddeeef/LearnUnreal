
namespace 
{
    
/**
* Definition for singly-linked list.
* */
struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(nullptr) {}
};

// 环形链表2，这次还需要返回交叉点
class Solution {
public:
    // 方法1：用hashset和
    // 方法2： 快慢指针，先找到快慢指针重合点，再找交叉点，根据推导有慢指针和从头指针开始的移动必然重叠在交叉点
    ListNode *detectCycle(ListNode *head) {

        ListNode* slowPtr = head;
        ListNode* fastPtr = head;

        while (fastPtr != nullptr)
        {
            slowPtr = slowPtr->next;
            if (fastPtr->next == nullptr)
            {
                return nullptr;
            }
            fastPtr = fastPtr->next->next;

            if (fastPtr == slowPtr)
            {
                ListNode* newPtr = head;

                while (newPtr != slowPtr)
                {
                    newPtr = newPtr->next;
                    slowPtr = slowPtr->next;
                }
                return newPtr;
            }
        }

        
        return nullptr;
    }
};
}
