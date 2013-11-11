#include <stdio.h>
#define ARRAY_SIZE 22

int find_min(int* array) {
    int last = ARRAY_SIZE - 1;
    int i = 0;
    int mid = last / 2;
    while(i < mid && mid < last) {
        printf("i: %d(%d), mid: %d(%d), last: %d(%d)\n", i, array[i], mid, array[mid], last, array[last]);
        if(array[i] < array[last]) return array[i];
        else if(array[i] < array[mid]) i = mid;
        else last = mid;
        mid = i + (last - i)/2;
    }
    return array[mid + 1];
}

int find_min_n(int* array) {
    int i = 1;
    for(; i < ARRAY_SIZE; ++i) {
        if(array[i] < array[i - 1]) break;
    }
    return array[i % (ARRAY_SIZE)];
}

int main() {
    //int array[] = {12, 15, 22, 56, 94, 3, 7, 11};
    int array[] = {30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 10, 20, 21, 22, 23, 24, 25, 26, 27, 29};
    //int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    printf("min of array is %d with O(logn)\n", find_min(array));
    printf("min of array is %d with O(n)\n", find_min_n(array));
    return 0;
}
