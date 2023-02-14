#include <vector>

namespace 
{
    

using namespace std;
class Solution {
public:
    
    void merge(vector<int>& nums1, int m, vector<int>& nums2, int n) {
        int iTarget = m + n - 1;
        int& iNum1 = --m;
        int& iNum2 = --n;

        while (iNum2 >= 0)
        {
            while (iNum1 >= 0 && nums1[iNum1] > nums2[iNum2])
            {
                swap(nums1[iTarget--], nums1[iNum1--]);
            }
            swap(nums1[iTarget--], nums2[iNum2--]);
        }
    }
};
}
