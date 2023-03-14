#include <algorithm>
#include <unordered_map>
#include <vector>

namespace
{
    //多数元素问题，假定输入的数组一定存在多数元素
    class Solution
    {
    public:
        // 用hash map
        int majorityElement(std::vector<int>& nums) {
            using namespace std;
            if (nums.empty())
            {
                return 0;
            }

            std::unordered_map<int, int> numMap;

            int count = 0;
            int maxElement = 0;
            for (int numElement : nums)
            {
                ++numMap[numElement];

                if (numMap[numElement] > count)
                {
                    maxElement = numElement;
                    count = numMap[numElement];
                }
            }
            return maxElement;
        }

        // 摩尔投票法
        int majorityElement2(std::vector<int>& nums)
        {
                if (nums.empty())
                {
                    return 0;
                }

                int curMaxNum = nums[0];
                int curCount = 1;

                for (int numElement : nums)
                {
                    if (numElement == curMaxNum)
                    {
                        curCount++;
                    }
                    else
                    {
                        curCount--;
                        if (curCount == 0) 
                        {
                            curMaxNum = numElement;
                            curCount = 1;
                        }
                    }
                }
                return  curMaxNum;
        
           
        }
        // 先排序，[n/2]一定是多元素
        int majorityElement1(std::vector<int>& nums) {
            {
                std::sort(nums.begin(), nums.end());

                return nums[nums.size()/2];
            }

        };
    };
}