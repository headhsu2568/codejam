/******
 *
 * 'NP simple CGI program'
 * by headhsu
 *
 * gcc -lfcgi -o simpleCGI.fcgi simpleCGI.c
 *
 ******/

#include "/usr/include/fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*** Define value ***/
#define BUF_MAX_SIZE 1024
#define PARAM_MAX_NUM 20
#define QRYSTR_MAX_LEN 1024

struct PARAM {
    char* paramKey;
    char* paramValue;
};

char* length = NULL;
int leng = 0;
char buffer[BUF_MAX_SIZE];
struct PARAM param[PARAM_MAX_NUM];
int paramNum = 0;
char queryString[QRYSTR_MAX_LEN];
char queryStringBak[QRYSTR_MAX_LEN];
int queryStringLen = 0;

void ParseParameter();

int main() {
    //length = getenv("CONTENT_LENGTH");
    //leng = atoi(length);
 
    int count = 0;
    while(FCGI_Accept() >= 0) {
        printf("Content-type: text/html\r\nStatus: 200 OK\r\n\r\n");
        printf("<title>FastCGI Hello! (C, fcgi_stdio library)</title>");
        printf("<h1>FastCGI Hello! (C, fcgi_stdio library)</h1>");
        printf("<br>Request number %d running on host <i>%s</i>", count, getenv("SERVER_NAME"));
        
        ParseParameter();
        printf("<br>Request has query string with %s", queryStringBak);

        ++count;
        if(count > 100) break;
    }

    FCGI_Finish();
}

/*** QUERY_STRING(GET parameters) parser ***/
void ParseParameter() {
    int queryStringLen = 0;
    int i = 0;
    if(strlen(getenv("QUERY_STRING")) > 0) {
        strncpy(queryString, getenv("QUERY_STRING"), QRYSTR_MAX_LEN);
        queryStringLen = strlen(queryString);
        strncpy(queryStringBak, queryString, queryStringLen);
        
        char* ptrTmp = queryString;
        if( (ptrTmp = strtok(ptrTmp, "&")) ) {
            param[i].paramKey = ptrTmp;
            ++i;
            for(; (ptrTmp = strtok(NULL, "&")); ++i) {
                param[i].paramKey = ptrTmp;
            }
            paramNum = i;
        }

        ptrTmp = NULL;
        for(i=0; i < paramNum; ++i) {
            ptrTmp = param[i].paramKey;
            if( (ptrTmp = strpbrk(ptrTmp, "=")) ) {
                strtok(param[i].paramKey, "=");
                param[i].paramValue = ptrTmp + 1;
            }
            else {
                write(STDOUT_FILENO, "<br>parse GET parameter error!\n", 31);
            }
            printf("<br>%s : %s", param[i].paramKey, param[i].paramValue);
        }
    }
    else {
        strcpy(queryString, "(null)");
    }
    return;
}
