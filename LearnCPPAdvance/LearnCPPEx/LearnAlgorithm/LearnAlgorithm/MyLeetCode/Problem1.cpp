
#include <algorithm>
#include <unordered_map>
#include <vector>


namespace 
{
    using namespace std;
    class Solution
    {
    public:
        vector<int> twoSum(vector<int>& nums, int target)
        {
            unordered_map<int, int> hashtable;
            for (int i = 0; i < nums.size(); ++i)
            {
                auto it = hashtable.find(target - nums[i]);
                if (it != hashtable.end())
                {
                    return {it->second, i};
                }
                hashtable.insert(make_pair(nums[i],i));
            }
            return {};
        }
    };
}
