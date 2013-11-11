/***
 * Search in a Rotation of an Array:
 *
 * When some elements at the beginning of an array are moved to the end, it gets a rotation of the original array.
 * Please implement a function to search a number in a rotation of an increasingly sorted array. Assume there are no duplicated numbers in the array.
 * For example, array {3, 4, 5, 1, 2} is a rotation of array {1, 2, 3, 4, 5}.
 * If the target number to be searched is 4, the index of the number 4 in the rotation 1 should be returned.
 * If the target number to be searched is 6, -1 should be returned because the number does not exist in the rotated array.
 *
 ***/
#include <iostream>

using namespace std;

int BinarySearch(int *ary, int n, int start, int end) {
    int mid = start + (end-start)/2;
    if(start > end) return -1;
    else if(n == ary[start]) return start;
    else if(n == ary[end]) return end;
    else if(n == ary[mid]) return mid;
    else if(n > ary[start] && n < ary[mid]) return BinarySearch(ary, n, start, mid);
    else return BinarySearch(ary, n, mid+1, end);
}

int SearchInARotationArray(int *ary, int n, int start, int end) {
    int mid = start + (end-start)/2;
    if(n == ary[start]) return start;
    else if(n == ary[end]) return end;
    else if(n == ary[mid]) return mid;
    else if(ary[mid] >= ary[start]) {
        if(n > ary[start] && n < ary[mid]) return BinarySearch(ary, n, start, mid);
        else return SearchInARotationArray(ary, n, mid+1, end);
    }
    else if(ary[end] >= ary[mid+1]) {
        if(n > ary[mid+1] && n < ary[end]) return BinarySearch(ary, n, mid+1, end);
        else return SearchInARotationArray(ary, n, start, mid);
    }
    return -1;
}

int main() {
    int ary[] = {3, 4, 5, 1, 2};
    int n = 7;
    cout << "Search: " << n << endl;
    cout << "Rotation: " << SearchInARotationArray(ary, n, 0, sizeof(ary)/sizeof(int)-1) << endl;
    return 0;
}
