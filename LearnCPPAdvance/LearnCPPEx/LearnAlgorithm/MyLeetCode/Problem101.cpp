
#include <queue>

namespace 
{

/**
* Definition for a binary tree node.
* */
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

using namespace std;
/**
 * \brief 对称二叉树
 */
class Solution {
    bool isSymmetric(TreeNode* left, TreeNode* right)
    {
        queue<TreeNode*> nodeQueue;
        nodeQueue.push(left);
        nodeQueue.push(right);

        while (!nodeQueue.empty())
        {
            // 比较头元素
            TreeNode* leftHead = nodeQueue.front();
            nodeQueue.pop();

            TreeNode* rightHead = nodeQueue.front();
            nodeQueue.pop();

            if (!leftHead && !rightHead)
            {
                continue;
            }
            if ( (!leftHead || !rightHead)
                || leftHead->val != rightHead->val
                )
            {
                return false;
            }

            nodeQueue.push(leftHead->left);
            nodeQueue.push(rightHead->right);

            nodeQueue.push(leftHead->right);
            nodeQueue.push(rightHead->left);
        }
        return true;
    }
    
public:
    bool isSymmetric(TreeNode* root) {
        return isSymmetric(root, root);
    }
};
        
}
