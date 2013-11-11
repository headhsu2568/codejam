/***
 * UVA-139 Telephone Tangles
 *
 * A large company wishes to monitor the cost of phone calls made by its personnel.
 * To achieve this the PABX logs, for each call, the number called (a string of up to 15 digits) and the duration in minutes.
 * Write a program to process this data and produce a report specifying each call and its cost, based on standard Telecom charges.
 *
 * International (IDD) numbers start with two zeroes (00) followed by a country code (1-3 digits) followed by a subscriber's number (4-10 digits).
 * National (STD) calls start with one zero (0) followed by an area code (1-5 digits) followed by the subscriber's number (4-7 digits).
 * The price of a call is determined by its destination and its duration.
 * Local calls start with any digit other than 0 and are free.
 *
 * Input will be in two parts.
 * The first part will be a table of IDD and STD codes, localities and prices as follows:
 *
 * Code * Locality name$price in cents per minute
 * where * represents a space.
 * Locality names are 25 characters or less.
 * This section is terminated by a line containing 6 zeroes (000000).
 *
 * The second part contains the log and will consist of a series of lines, one for each call, containing the number dialled and the duration.
 * The file will be terminated a line containing a single #.
 * The numbers will not necessarily be tabulated, although there will be at least one space between them.
 * Telephone numbers will not be ambiguous.
 *
 * Output will consist of the called number, the country or area called, the subscriber's number, the duration, the cost per minute and the total cost of the call, as shown below.
 * Local calls are costed at zero.
 * If the number has an invalid code, list the area as ``Unknown'' and the cost as -1.00.
 *
 ***/
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class RECORD {
    public:
        RECORD(char* n, int p);
        char name[25];
        int price;
};
RECORD::RECORD(char* n, int p) : price(p) {
    strcpy(name, n);
}

class NUM {
    public:
        NUM();
        RECORD* r;
        NUM* ary[10];
};
NUM::NUM() : r(NULL) {
    int i = 0;
    for(; i < 10; ++i) ary[i] = NULL;
}

NUM* table;

int InsertRecord() {
    char str[1024];
    gets(str);
    if(!strcmp(str, "000000")) return 0;

    NUM* t_ptr = table;
    char* s_ptr = str;
    while(*s_ptr != ' ') {
        int n = *s_ptr - '0';
        if(t_ptr->ary[n] == NULL) t_ptr->ary[n] = new NUM();
        t_ptr = t_ptr->ary[n];
        //cout << n << " ";
        s_ptr++;
    }
    s_ptr++;

    char* s_ptr2 = s_ptr;
    while(*s_ptr2 != '$') s_ptr2++;
    *s_ptr2++ = '\0';

    t_ptr->r = new RECORD(s_ptr, atoi(s_ptr2));
    //cout << ":" << t_ptr->r->name << ", " << t_ptr->r->price << endl;

    return 1;
}

int SearchCall() {
    char str[1024];
    gets(str);
    if(!strcmp(str, "#")) return 0;

    char number[1024];
    int time;
    int price;

    sscanf(str, "%s %d", number, &time);
    if(number[0] != '0') {
        printf("%s\tLocal\t%s\t%d\t0.00\t0.00\n", number, number, time);
        return 1;
    }

    NUM* t_ptr = table;
    char* s_ptr = number;
    //cout << "Search:" << endl;
    while(*s_ptr != '\0' && t_ptr != NULL && t_ptr->r == NULL) {
        int i = *s_ptr-'0';
        t_ptr = t_ptr->ary[i];
        //cout << i << " ";
        s_ptr++;
    }
    if(t_ptr == NULL) {
        printf("%s\tUnknown\t\t\t%d\t\t\t-1.00\n", number, time);
        return 1;
    }

    //cout << ":" << t_ptr->r->name << ", " << t_ptr->r->price << endl;

    char* s_ptr2 = s_ptr;
    while(*s_ptr2 != '\0') s_ptr2++;
    int len = s_ptr2-s_ptr;

    if(number[0] == '0' && number[1] == '0' && (len < 4 || len > 10)) {
        printf("%s\tUnknown\t\t\t%d\t\t\t-1.00\n", number, time);
        return 1;
    }
    else if(number[0] == '0' && (len < 4 || len > 7)) {
        printf("%s\tUnknown\t\t\t%d\t\t\t-1.00\n", number, time);
        return 1;
    }

    printf("%s\t%s\t%s\t%d\t%f\t%f\n", number, t_ptr->r->name, s_ptr, time, t_ptr->r->price/100.0, time*t_ptr->r->price/100.0);
    return 1;
}

int main() {
    table = new NUM();
    while(InsertRecord());

    while(SearchCall());

    return 0;
}
