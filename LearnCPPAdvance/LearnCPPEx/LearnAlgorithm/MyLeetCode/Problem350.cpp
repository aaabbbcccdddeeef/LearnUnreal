
#include <unordered_map>
#include <vector>

namespace 
{
    
}
using namespace std;

class Solution {
public:
    vector<int> intersect(vector<int>& nums1, vector<int>& nums2) {
        if (nums1.size() > nums2.size())
        {
            intersect(nums2, nums1);
        }

        unordered_map<int, int> num1HashMap;
        for (int element : nums1)
        {
            num1HashMap[element]++; 
        }

        vector<int> result;
        for (int num2Element : nums2)
        {
            if (num1HashMap.count(num2Element))
            {
                num1HashMap[num2Element]--;
                result.push_back(num2Element);
                if (num1HashMap[num2Element] <= 0)
                {
                    num1HashMap.erase(num2Element);
                }
            }
        }
        return result;
    }
};