
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace 
{
    using namespace std;
    class Solution
    {
    public:
        bool containsDuplicate(vector<int>& nums)
        {
            unordered_set<int> tmpMap;
            for (int element : nums)
            {
                if (tmpMap.find(element) != tmpMap.end())
                {
                    return true;
                }
                tmpMap.insert(element);
            }
            return false;
        }

        bool containsDuplicate2(vector<int>& nums)
        {
            sort(nums.begin(), nums.end());
            int i = 0;
            for (; i + 1 < nums.size(); ++i)
            {
                if (nums[i] == nums[i+1])
                {
                    return true;
                }
            }
            return false;
        }
    };
}
