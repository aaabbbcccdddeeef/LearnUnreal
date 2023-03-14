
namespace 
{

/**
* Definition for singly-linked list.
* */
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

// 合并2个有序单链表
class Solution {
public:
    ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {

        ListNode* preHead = new ListNode(-1);

        ListNode* curNode = preHead;
        while (list1 && list2)
        {
            if (list1->val < list2->val)
            {
                curNode->next = list1;
                list1 = list1->next;
            }
            else
            {
                curNode->next = list2;
                list2 = list2->next;
            }
            curNode = curNode->next;
        }
        curNode->next = list1 == nullptr ? list2 : list1;

        return preHead->next;

    }
};    
}
