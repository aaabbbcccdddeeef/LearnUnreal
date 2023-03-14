#include <algorithm>

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

    // 二叉树的直径
    class Solution {
    private:
        int diameter;
        
        int depth(TreeNode* inNode)
        {
            if (inNode == nullptr)
            {
                return 0;
            }

            int leftDepth = depth(inNode->left);
            int rightDepth = depth(inNode->right);
            diameter = std::max(leftDepth + rightDepth, diameter);

            return std::max(leftDepth, rightDepth) + 1;
        }
        
    public:
        int diameterOfBinaryTree(TreeNode* root) {

            depth(root);
            return diameter;
        }
    };
}
