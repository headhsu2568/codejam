#include <stdio.h>

int Get_Bit7_of_Input(int in) {
    return (in & 64) >> 6;
}
void Set_Bit5_of_Input(int *in) {
    (*in) |= 16;
}
void Clear_Bit_of_Input(int *in, int bit) {
    (*in) &= ~(1 << (bit-1));
}

int main() {
    int in1 = 65;
    printf("bit 7 of %d is %d\n", in1, Get_Bit7_of_Input(in1));

    int in2 = 0;
    int in2_set = in2;
    Set_Bit5_of_Input(&in2_set);
    printf("set bit 5 of %d to %d\n", in2, in2_set);

    int in3 = 9;
    int in3_clear = in3;
    int bit = 4;
    Clear_Bit_of_Input(&in3_clear, bit);
    printf("clear bit %d of %d to %d\n", bit, in3, in3_clear);
    return 0;
}
