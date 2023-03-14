
#include <unordered_set>

namespace
{
    
}
/**
* Definition for singly-linked list.
* */
struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(nullptr) {}
};

class Solution {
public:
    ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
        std::unordered_set<ListNode*> hashSet;

        ListNode* tmp = headA;
        while (tmp)
        {
            hashSet.emplace(tmp);
            tmp = tmp->next;
        }

        tmp = headB;
        while (tmp)
        {
            if (hashSet.count(tmp))
            {
                return tmp;
            }

            tmp = tmp->next;
        }
        return nullptr;
    }
};