
#include <algorithm>
#include <unordered_map>
#include <vector>


namespace 
{
    using namespace std;
    class Solution
    {
    public:
        // 要求输入是有序的vector
        int removeReplicates(vector<int>& nums)
        {
            if (nums.empty())
            {
                return  0;
            }
            
            // front ptr 记录不重复的数值的最后index
            int frontPtr = 0;
            int backPtr = 1;
            while (backPtr < nums.size())
            {
                if (nums[backPtr] != nums[frontPtr])
                {
                    frontPtr++;
                    nums[frontPtr] = nums[backPtr];
                }
                backPtr++;
            }
            return frontPtr + 1;
        }

        
    };
}
