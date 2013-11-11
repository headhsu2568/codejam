#include <stdio.h>
#include <string.h>
#define MAX_LEN 1024

void str_reverse(char* str, int pos) {
    int len = strlen(str);
    int s = 0;
    int d = pos;
    for(; s < d ; ++s, --d) {
        str[s] ^= str[d];
        str[d] ^= str[s];
        str[s] ^= str[d];
    }
    for(s = pos+1, d = len-1; s < d; ++s, --d) {
        str[s] ^= str[d];
        str[d] ^= str[s];
        str[s] ^= str[d];
    }
}

int strcmp2(char *source, char * dest) {
    int i = 0;
    for(; source[i] == dest[i]; ++i) {
        if(source[i] == '\0' && dest[i] == '\0') return 0;
    }
    return -1;
}

int main() {
    const char* source = "abcdefghijklmnopqrstuvwxyz1234567890";
    printf("source: %s\n", source);

    char str[MAX_LEN];
    strncpy(str, source, MAX_LEN-1);

    int pos = 25;
    str_reverse(str, pos);
    printf("result: %s\n", str);

    char str1[] = "abcdefghijklmnopqrstuvwxyz1234567890";
    char str2[] = "abcdefghijklmnopqrstuvwxyz1234567890";
    printf("str1: %s\nstr2: %s\nstrcmp2: %d\n", str1, str2, strcmp2(str1, str2));
    return 0;
}
