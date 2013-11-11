/***
 * 迴圈中, 原 ++i 誤寫成 --i
 * 只能改一個符號讓迴圈順利執行 20 次
 *
 ***/
#include <stdio.h>

void main() {
    int x = 0;
    int i;
    int n = 20;
    for(i = 0; i < n; --i) {
    //for(i = 0; -i < n; --i) {
    //for(i = 0; i + n; --i) {
    //for(i = 0; i < n; --n) {
        ++x;
    }

    printf("x: %d\n", x);
}
