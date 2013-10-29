/******
 *
 * 'NP hw3 part3 - part1 cgi program & part2 web server with winsock version'
 * by headhsu
 *
 ******/
#include <windows.h>
#include <list>
#include <string.h>
#include "resource.h"
using namespace std;

/*** Define value ***/
#define BUF_MAX_SIZE 10240
#define WRITE_MAX_SIZE 20480
#define HEADER_MAX_LEN 1024
#define HEADER_NUM 9
#define CONNECTION_MAX_NUM 10
#define ENVPARAM_NUM 9
#define ENVPARAM_MAX_LEN 1500
#define PARAM_MAX_NUM 30
#define BATCHFILE_NAME_MAX_LEN 20
#define SERVER_PORT 7775
#define WM_SOCKET_NOTIFY (WM_USER + 1)
#define WM_CLIENT_NOTIFY (WM_USER + 2)
#define WM_CGI_NOTIFY (WM_USER + 3)

/*** Define request header index ***/
#define METHOD 0
#define HOST 1
#define CONNECTION 2
#define USERAGENT 3
#define ACCEPT 4
#define ACCEPTENCODING 5
#define ACCEPTLANGUAGE 6
#define ACCEPTCHARSET 7
#define OTHER 8

/*** Define environment parameters(envParam[]) index ***/
#define QUERY_STRING 0
#define CONTENT_LENGTH 1
#define REQUEST_METHOD 2
#define SCRIPT_NAME 3
#define REMOTE_HOST 4
#define REMOTE_ADDR 5
#define ANTH_TYPE 6
#define REMOTE_USER 7
#define REMOTE_IDENT 8

/*** Define part1 cgi program connection status code ***/
#define CONNECTING 1
#define READING 2
#define WRITING 3
#define DONE 4

/*** Define web server client status code ***/
#define INITIAL 0
#define REQUEST 1

/*** Define function for Unix compatible to Windows ***/
#define bzero(a, b) memset(a, 0, b)

//=================================================================
//	Data structure definition
//=================================================================
struct ClientInfo {

	/*** connection variables ***/
	int fd;
	int clilen;
	struct sockaddr_in cli_addr;
	unsigned int port;
	char ip[16];

	/*** user environment ***/
	int status;
	int isConnect;
};
struct PARAM {
	char* paramKey;
	char* paramValue;
};
struct CONN {
	struct sockaddr_in conn_addr;
	int connfd;
	int status;
	char* no;
	char* ip;
	char* port;
	char* batchFile;
	FILE* fp;
	int clientfd;
	int readable;
	int writable;
	int pipeEofFSM;
	int changeOption;
};

//=================================================================
//	Functions declare
//=================================================================
/*** main function ***/
BOOL CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
int EditPrintf (HWND, TCHAR *, ...);
int Write(int fd, char* formatString, ...);
LPVOID StrError();

/*** web server level function ***/
void ClientNotifyHandler(WPARAM wParam, LPARAM lParam);
void NewClient();
void FindCurrentClient(int fd);
int ReadRequest();
void ParseMethod(char* method, char* target, char* queryString);
void ExecuteMethod(char* method, char* target, char* queryString);
void Disconnect();
void ListRequest();

/**** part1 cgi program level function **/
void CgiNotifyHandler(WPARAM wParam, LPARAM lParam);
void ExecuteCgi(char* method, char* target, char* queryString, int clientNo);
void Initialization();
void SetEnvParam(char* method, char* target, char* queryString);
void ParseParameter();
void SetConnVar(int clientNo);
void CreateConnectionForConn();
int  FindCurrentConn(int fd);
void DisconnectConn(int connNo);
void ReadFromConn(int connNo);
void WriteToConn(int connNo);
void ListAllConnVar();

//=================================================================
//	Global Variables
//=================================================================
/*** environment parameters ***/
LPVOID lastErrorString;
HWND hwndEdit;
HWND curHwnd;

/*** environment parameters in a web server ***/
int clientCount = 0;
int maxClientNo = -1;
int curClientNo = -1;
struct ClientInfo curClient[CONNECTION_MAX_NUM];
/*** connection variables ***/
SOCKET listenfd, connfd;
int clilen;
struct sockaddr_in serv_addr, cli_addr;
char request[HEADER_NUM][HEADER_MAX_LEN];
char buffer[BUF_MAX_SIZE];

/*** environment parameters in a cgi program ***/
int cgiUsing = 0;
char envParam[ENVPARAM_NUM][ENVPARAM_MAX_LEN];
struct PARAM param[PARAM_MAX_NUM];
int paramNum = 0;
char queryString[ENVPARAM_MAX_LEN];
char queryStringBak[ENVPARAM_MAX_LEN];
int queryStringLen = 0;
int connNum = 0;
struct CONN conn[PARAM_MAX_NUM];
int activeConnNum = 0;
int curConnNo = -1;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	return DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, MainDlgProc);
}

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	WSADATA wsaData;
	int err, i = 0;

	switch(Message) {
	case WM_INITDIALOG:
		hwndEdit = GetDlgItem(hwnd, IDC_RESULT);
		curHwnd = hwnd;
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case ID_LISTEN:

			WSAStartup(MAKEWORD(2, 0), &wsaData);

			/*** initialize all ClientInfo ***/
			for(i = 0; i < CONNECTION_MAX_NUM; ++i) {
				curClient[i].fd = -1;
			}

			//create master socket
			listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if( listenfd == INVALID_SOCKET ) {
				EditPrintf(hwndEdit, TEXT("=== Error: create socket error ===\r\n"));
				WSACleanup();
				return TRUE;
			}

			err = WSAAsyncSelect(listenfd, hwnd, WM_SOCKET_NOTIFY, FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE);

			if ( err == SOCKET_ERROR ) {
				EditPrintf(hwndEdit, TEXT("=== Error: select error ===\r\n"));
				closesocket(listenfd);
				WSACleanup();
				return TRUE;
			}

			//fill the address info about server
			serv_addr.sin_family		= AF_INET;
			serv_addr.sin_port			= htons(SERVER_PORT);
			serv_addr.sin_addr.s_addr	= INADDR_ANY;

			//bind socket
			err = bind(listenfd, (LPSOCKADDR)&serv_addr, sizeof(struct sockaddr));

			if( err == SOCKET_ERROR ) {
				EditPrintf(hwndEdit, TEXT("=== Error: binding error ===\r\n"));
				WSACleanup();
				return FALSE;
			}

			err = listen(listenfd, 2);

			if( err == SOCKET_ERROR ) {
				EditPrintf(hwndEdit, TEXT("=== Error: listen error ===\r\n"));
				WSACleanup();
				return FALSE;
			}
			else {
				EditPrintf(hwndEdit, TEXT("=== Server START ===\r\n"));
			}
			break;

		case ID_EXIT:
			EndDialog(hwnd, 0);
			break;
		};
		break;// end of case WM_COMMAND

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;

	case WM_SOCKET_NOTIFY:
		switch( WSAGETSELECTEVENT(lParam) ) {
		case FD_ACCEPT:

			/*** initial the client ***/
			clilen = sizeof(cli_addr);
			connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
			NewClient();
			if(curClientNo == -1) Disconnect();

			WSAAsyncSelect(connfd, hwnd, WM_CLIENT_NOTIFY, FD_CLOSE | FD_READ | FD_WRITE);
			break;

		case FD_READ:
			//Write your code for read event here.
			break;

		case FD_WRITE:
			//Write your code for write event here.
			break;

		case FD_CLOSE:
			break;
		};
		break;// end of case WM_SOCKET_NOTIFY

	case WM_CLIENT_NOTIFY:
		ClientNotifyHandler(wParam, lParam);
		break;

	case WM_CGI_NOTIFY:
		CgiNotifyHandler(wParam, lParam);
		break;

	default:
		return FALSE;
	}; // end of switch(message)
	return TRUE;
} // end of MainDlgProc()

int EditPrintf (HWND hwndEdit, TCHAR * szFormat, ...)
{
	TCHAR   szBuffer [WRITE_MAX_SIZE] ;
	va_list pArgList ;

	va_start (pArgList, szFormat) ;
	vsprintf(szBuffer, szFormat, pArgList);
	va_end (pArgList) ;

	SendMessage (hwndEdit, EM_SETSEL, (WPARAM) -1, (LPARAM) -1) ;
	SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) szBuffer) ;
	SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;
	return SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0); 
}

int Write(int fd, char* formatString, ...) {
	char buf[WRITE_MAX_SIZE];
	bzero(buf, WRITE_MAX_SIZE);
	va_list pArgList;

	va_start(pArgList, TEXT(formatString));
	vsprintf(buf, TEXT(formatString), pArgList);
	va_end(pArgList);

	int writeLen = strlen(buf);
	int n  = send(fd, buf, writeLen, 0);
	//EditPrintf(hwndEdit, TEXT("send %d bytes, success %d bytes\n"), writeLen, n);
	return writeLen;
}

LPVOID StrError() {
	LocalFree(lastErrorString);
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lastErrorString,
		0,
		NULL);
	strtok((char*)lastErrorString, "\n");
	return lastErrorString;
}

/*** the handler for handling the WM_CLIENT_NOTIFY event that driven by WSAAsyncSelect ***/
void ClientNotifyHandler(WPARAM wParam, LPARAM lParam) {
	FindCurrentClient(wParam);
	switch( WSAGETSELECTEVENT(lParam) ) {
	case FD_READ:
		if(curClient[curClientNo].status == INITIAL) {
			curClient[curClientNo].status = REQUEST;

			/*** read, parse request like http server ***/
			ReadRequest();

			/*** send status response ***/
			Write(curClient[curClientNo].fd, "HTTP/1.1 200 OK");

			/*** parse headers ***/
			char method[5];
			char target[HEADER_MAX_LEN];
			char queryString[HEADER_MAX_LEN];
			ParseMethod(method, target, queryString);

			/*** execute method (do something like cgi program) ***/
			ExecuteMethod(method, target, queryString);
		}
		break;

	case FD_WRITE:
		break;

	case FD_CLOSE:
		Disconnect();
		break;
	}
	return;
}

/*** initialize a new client after accepting a connection ***/
void NewClient() {

	int i = 0;
	for(; i < CONNECTION_MAX_NUM; ++i) {
		if(curClient[i].fd != -1) continue;

		/*** initialize client (struct ClientInfo) ***/
		curClient[i].fd = connfd;
		curClient[i].clilen = clilen;
		curClient[i].cli_addr = cli_addr;
		curClient[i].port = htons(curClient[i].cli_addr.sin_port);
		strncpy(curClient[i].ip, inet_ntoa(curClient[i].cli_addr.sin_addr), 16);
		curClient[i].isConnect = 1;
		curClient[i].status = INITIAL;

		++clientCount;
		curClientNo = i;
		if(i > maxClientNo) maxClientNo = i;

		EditPrintf(hwndEdit, TEXT("new client #%d (fd: %d, online: %d)\n"), i, curClient[i].fd, clientCount);
		break;
	}
	if(i == CONNECTION_MAX_NUM) {
		EditPrintf(hwndEdit, TEXT("error: too many client now\n"));
		curClientNo = -1;
	}
	return;
}

/*** when receive notification, find the client fd ***/
void FindCurrentClient(int fd) {
	int i = 0;
	for(; i <= maxClientNo; ++i) {
		if (fd == curClient[i].fd) {
			curClientNo = i;
			//EditPrintf(hwndEdit, TEXT("current client at %d, fd is %d\n"), curClientNo, fd);
			break;
		}
	}
	if (i > maxClientNo) {
		//EditPrintf(hwndEdit, TEXT("error: cannot find client fd %d\n"), fd);
	}
	return;
}

/*** read request from client and store to array ***/
int ReadRequest() {
	int n = 0, i = 0;
	char* ptrBuf = buffer;
	if( (n = recv(curClient[curClientNo].fd, buffer, BUF_MAX_SIZE-1, 0)) > 0) {
		buffer[n] = '\0';
		//EditPrintf(hwndEdit, TEXT("read request %d bytes\n"), n);
		//EditPrintf(hwndEdit, TEXT("request: \n"));
		//EditPrintf(hwndEdit, TEXT("%s\n"), buffer);

		if((ptrBuf = strtok(ptrBuf, "\n"))) {
			strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);
			++i;
			while((ptrBuf = strtok(NULL, "\n")) && i < HEADER_NUM-1) {
				strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);
				++i;
			}
			if(i == HEADER_NUM-1) {
				strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);
				++i;
			}
		}
		else strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);

		//ListRequest();
		bzero(buffer, BUF_MAX_SIZE);
	}
	else if(n == 0) {
		Disconnect();
		return -1;
	}
	else if (n < 0) {
		EditPrintf(hwndEdit, TEXT("error: read error (%d - %s)\n"), GetLastError(), (char*)StrError());
		return -1;
	}
	return 0;
}

/*** parse the method header after read request ***/
void ParseMethod(char* method, char* target, char* queryString) {
	char tmpReq[HEADER_MAX_LEN];
	bzero(tmpReq, HEADER_MAX_LEN);
	strncpy(tmpReq, request[METHOD], HEADER_MAX_LEN);
	char* ptrReq = tmpReq;
	if((ptrReq = strtok(ptrReq, " "))) {
		strncpy(method, ptrReq, 4);
		//EditPrintf(hwndEdit, TEXT("method: %s\n"), method);

		if((ptrReq = strtok(NULL, " "))) {
			char* ptrTar = ptrReq;

			ptrReq = strtok(ptrReq, "?");
			if((ptrReq = strtok(NULL, "?"))) {
				strcpy(target, ptrTar);
				//EditPrintf(hwndEdit, TEXT("target: %s\n"), target);
				strcpy(queryString, ptrReq);
				//EditPrintf(hwndEdit, TEXT("query string: %s\n"), queryString);
			}
			else {
				strcpy(target, ptrTar);
				//EditPrintf(hwndEdit, TEXT("target: %s\n", target));
				strcpy(queryString, "");
				//EditPrintf(hwndEdit, TEXT("query string: %s(empty)\n"), queryString);
			}
		}
	}
	return;
}

/*** execute the action of method header ***/
void ExecuteMethod(char* method, char* target, char* queryString) {
	char* targetExt = strrchr(target, '.');
	char* targetPath = target;
	int endOption = 1;

	if(strcmp(method, "GET") == 0) {
		/*** method: GET ***/

		/*** parse target file extension and execute ***/
		if(targetExt != NULL) {
			if((strcmp(targetExt, ".cgi") == 0) || (strcmp(targetExt, ".fcgi") == 0)) {

				if(cgiUsing == 0) {
					cgiUsing = 1;
					EditPrintf(hwndEdit, TEXT("execute %s program\n"), targetExt);

					/*** execute part1 cgi program ***/
					endOption = 0;
					ExecuteCgi(method, target, queryString, curClientNo);
				}
				else {
					EditPrintf(hwndEdit, TEXT("error: a cgi program is executing now, cannot execute %s program\n"), targetExt);
				}
			}
			else if((strcmp(targetExt, ".html") == 0) || strcmp(targetExt, ".htm") == 0) {

				/*** send response headers ***/
				//write(curClient.fd, responseHeader, strlen(responseHeader));

				/*** send response content ***/
				FILE* fp;
				if(fp = fopen(targetPath, "r")) {
					//printf("output %s web page\n", targetPath);fflush(stdout);
					char buf[BUF_MAX_SIZE];
					int n = 0;
					while(!feof(fp)) {
						n = fread(buf, sizeof(char), BUF_MAX_SIZE, fp);
						if(n <= 0) break;
						//write(curClient.fd, buf, n);
						bzero(buf, BUF_MAX_SIZE);
					}
					fclose(fp);
				}
				else {
					EditPrintf(hwndEdit, TEXT("error: open file failed (%d - %s)\n"), GetLastError(), (char*)StrError());
				}
			}
			else{
			}
		}
		else {
		}
	}
	else if(strcmp(method, "POST") == 0){
		/*** method: POST ***/
	}
	else {
		EditPrintf(hwndEdit, TEXT("error: unknown method\n"));
	}

	if(endOption) Disconnect();
	return;
}

/*** close the connection of the current client ***/
void Disconnect() {
	if(curClient[curClientNo].fd != -1) {
		closesocket(curClient[curClientNo].fd);
		--clientCount;
		EditPrintf(hwndEdit, TEXT("client #%d left (fd: %d, online: %d)\n"), curClientNo, curClient[curClientNo].fd, clientCount);
		bzero(&curClient[curClientNo], sizeof(curClient[curClientNo]));
		curClient[curClientNo].fd = -1;
		bzero(&cli_addr, sizeof(cli_addr));
		clilen = 0;
		connfd = 0;
		bzero(buffer, BUF_MAX_SIZE);
		bzero(request, sizeof(request));
	}
	return;
}

/*** show request ***/
void ListRequest() {
	int i = 0;
	EditPrintf(hwndEdit, TEXT("\n"));
	for(; i < HEADER_NUM; ++i) {
		if(strlen(request[i]) == 0) continue;
		EditPrintf(hwndEdit, TEXT("%d: %s\n"), i, request[i]);
	}
	return;
}

/*** part1 cgi program with winsock version ***/
void ExecuteCgi(char* method, char* target, char* querystring, int clientNo) {

	Initialization();
		
	/*** set environment parameters ***/
	SetEnvParam(method, target, querystring);

	/*** parse QUERY_STRING ***/
	ParseParameter();

	/*** set connection info ***/
	SetConnVar(clientNo);

	/*** create connections with connection info ***/
	CreateConnectionForConn();

	/*** send html text for the end ***/
	Write(curClient[curClientNo].fd, "</font>\n");
	Write(curClient[curClientNo].fd, "</body>\n");
	Write(curClient[curClientNo].fd, "</html>\n");
	return;
}

/*** when new client come, initial all global variables to default ***/
void Initialization() {
	bzero(envParam, sizeof(envParam));
	bzero(param, sizeof(param));
	bzero(conn, sizeof(conn));
	bzero(queryString, ENVPARAM_MAX_LEN);
	bzero(queryStringBak, ENVPARAM_MAX_LEN);
	paramNum = 0;
	queryStringLen = 0;
	connNum = 0;
	activeConnNum = 0;

	/*** send html text at first***/
	Write(curClient[curClientNo].fd, "Content-type: text/html\r\nStatus: 200 OK\r\n\r\n");
	Write(curClient[curClientNo].fd, "<html>\n");
	Write(curClient[curClientNo].fd, "<head>\n");
	Write(curClient[curClientNo].fd, "<meta http-equiv='Content-Type' content='text/html; charset=big5' />\n");
	Write(curClient[curClientNo].fd, "<title>NP homework3 part1 - CGI by ychsu</title>\n");
	Write(curClient[curClientNo].fd, "<h1>NP homework3 part3 - winsock version CGI by ychsu</h1>\n");
	Write(curClient[curClientNo].fd, "</head>\n");
	Write(curClient[curClientNo].fd, "<body bgcolor=#336699>\n");
	return;
}

/*** set environment parameters (envParam[]) for part1 cgi program ***/
void SetEnvParam(char* method, char* target, char* queryString) {

	/*** QUERY_STRING ***/
	strncpy(envParam[QUERY_STRING], queryString, ENVPARAM_MAX_LEN-1);

	/*** CONTENT_LENGTH ***/
	strncpy(envParam[CONTENT_LENGTH], "32767", ENVPARAM_MAX_LEN-1);

	/*** REQUEST_METHOD ***/
	strncpy(envParam[REQUEST_METHOD], method, ENVPARAM_MAX_LEN-1);

	/*** SCRIPT_NAME ***/
	strncpy(envParam[SCRIPT_NAME], target, ENVPARAM_MAX_LEN-1);

	/*** REMOTE_HOST ***/
	strncpy(envParam[REMOTE_HOST], "LOCALHOST", ENVPARAM_MAX_LEN-1);

	/*** REMOTE_ADDR ***/
	strncpy(envParam[REMOTE_ADDR], curClient[curClientNo].ip, ENVPARAM_MAX_LEN-1);

	/*** ANTH_TYPE ***/
	strncpy(envParam[ANTH_TYPE], "yes", ENVPARAM_MAX_LEN-1);

	/*** REMOTE_USER ***/
	strncpy(envParam[REMOTE_USER], "Nick", ENVPARAM_MAX_LEN-1);

	/*** REMOTE_IDENT ***/
	strncpy(envParam[REMOTE_IDENT], curClient[curClientNo].ip, ENVPARAM_MAX_LEN-1);
	return;
}

/*** QUERY_STRING(GET parameters) parser ***/
void ParseParameter() {
	int i = 0;
	if( (queryStringLen = strlen(envParam[QUERY_STRING])) > 0) {
		strncpy(queryString, envParam[QUERY_STRING], queryStringLen);
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
		for(i = 0; i < paramNum; ++i) {
			ptrTmp = param[i].paramKey;
			if( (ptrTmp = strpbrk(ptrTmp, "=")) ) {
				strtok(param[i].paramKey, "=");
				param[i].paramValue = ptrTmp + 1;
			}
			else {
				EditPrintf(hwndEdit, TEXT("parse GET parameter error!\n"));
			}
			//EditPrintf(hwndEdit, TEXT("%s : %s\n"), param[i].paramKey, param[i].paramValue);
		}
	}
	else {
		strcpy(queryString, "(null)");
		strcpy(queryStringBak, "(null)");
	}
	EditPrintf(hwndEdit, TEXT("Request has query string with %s\n"), queryStringBak);
	return;
}

/*** set variables for each conn (an array of struct CONN) ***/
void SetConnVar(int clientNo) {
    int i = 0, j = 0;
    connNum = 0;
    for(i = 0; i < paramNum; ++i) {
        if(strlen(param[i].paramValue) < 1) continue;
        if(*(param[i].paramKey) == 'h') {
            /*** set no. and ip ***/

            for(j = 0; j < connNum; ++j) {
                if(*(param[i].paramKey + 1) == *(conn[j].no)) {
                    conn[j].ip = param[i].paramValue;
                    break;
                }
            }
            if(j == connNum) {
                conn[connNum].no = param[i].paramKey + 1;
				conn[connNum].connfd = -1;
                conn[connNum].ip = param[i].paramValue;
				conn[connNum].port = "(null)";
				conn[connNum].batchFile = "(null)";
				conn[connNum].fp = NULL;
				conn[connNum].clientfd = curClient[clientNo].fd;
				conn[connNum].writable = 0;
				conn[connNum].readable = 0;
				conn[connNum].pipeEofFSM = 0;
				conn[connNum].changeOption = 0;
                ++connNum;
            }
        }
        else if(*(param[i].paramKey) == 'p') {
            /*** set no. and port ***/

            for(j = 0; j < connNum; ++j) {
                if(*(param[i].paramKey + 1) == *(conn[j].no)) {
                    conn[j].port = param[i].paramValue;
                    break;
                }
            }
            if(j == connNum) {
                conn[connNum].no = param[i].paramKey + 1;
                conn[connNum].connfd = -1;
				conn[connNum].ip = "(null)";
				conn[connNum].port = param[i].paramValue;
				conn[connNum].batchFile = "(null)";
				conn[connNum].fp = NULL;
				conn[connNum].clientfd = curClient[clientNo].fd;
				conn[connNum].writable = 0;
				conn[connNum].readable = 0;
				conn[connNum].pipeEofFSM = 0;
				conn[connNum].changeOption = 0;
                ++connNum;
            }
        }
        else if(*(param[i].paramKey) == 'f') {
            /*** set no. and batch file name ***/

            for(j = 0; j < connNum; ++j) {
                if(*(param[i].paramKey + 1) == *(conn[j].no)) {
                    conn[j].batchFile = param[i].paramValue;
                    if(conn[j].fp = fopen(conn[j].batchFile, "r")) {
                        //EditPrintf(hwndEdit, TEXT("open file: %s\n"), conn[j].batchFile);
                    }
                    else {
                        conn[j].fp = NULL;
                        EditPrintf(hwndEdit, TEXT("error: open file %s failed (%d-%s)\n"), conn[j].batchFile, GetLastError(), (char*)StrError());
                    }
                    break;
                }
            }
            if(j == connNum) {
                conn[connNum].no = param[i].paramKey + 1;
				conn[connNum].connfd = -1;
				conn[connNum].ip = "(null)";
				conn[connNum].port = "(null)";
                conn[connNum].batchFile = param[i].paramValue;
				conn[connNum].clientfd = curClient[clientNo].fd;
				conn[connNum].writable = 0;
				conn[connNum].readable = 0;
				conn[connNum].pipeEofFSM = 0;
				conn[connNum].changeOption = 0;
                if(conn[connNum].fp = fopen(conn[connNum].batchFile, "r")) {
                    //EditPrintf(hwndEdit, TEXT("open file: %s\n"), conn[j].batchFile);
                }
                else {
                    conn[connNum].fp = NULL;
                    EditPrintf(hwndEdit, TEXT("error: open file %s failed (%d-%s)\n"), conn[j].batchFile, GetLastError(), (char*)StrError());
                }
                ++connNum;
            }
        }
    }
   // ListAllConnVar();
    return;
}

/*** after setting each conn variables, create these connection to specific server ***/
void CreateConnectionForConn() {
	int i = 0, res = 0;

    /*** html table thead ***/
	Write(curClient[curClientNo].fd, "<font face='Courier New' size=3 color=#FFFF99>\n");
	Write(curClient[curClientNo].fd, "<table width='1024' border='1'>\n");
	Write(curClient[curClientNo].fd, "<tr>\n");
	for(i = 0; i < connNum; ++i) {

		/*** create socket ***/
        conn[i].conn_addr.sin_family = AF_INET;
        conn[i].conn_addr.sin_addr.s_addr = inet_addr(conn[i].ip);
        conn[i].conn_addr.sin_port = htons(atoi(conn[i].port));
		conn[i].connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if( conn[i].connfd == INVALID_SOCKET ) {
			EditPrintf(hwndEdit, TEXT("error: cannot open socket (%d - %s)\n"), GetLastError(), (char*)StrError());
			conn[i].connfd = -1;
			continue;
		}
		//EditPrintf(hwndEdit, TEXT("create socket fd: %d\n"), conn[i].connfd);

		/*** set asynchronous select ***/
		res = WSAAsyncSelect(conn[i].connfd, curHwnd, WM_CGI_NOTIFY, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
		if ( res == SOCKET_ERROR ) {
			EditPrintf(hwndEdit, TEXT("error: select failed (%d - %s)\n"), GetLastError(), (char*)StrError());
			closesocket(conn[i].connfd);
			conn[i].connfd = -1;
			continue;
		}

		/*** create connection ***/
		res = connect(conn[i].connfd, (SOCKADDR *)&conn[i].conn_addr, sizeof(conn[i].conn_addr));
		if(res < 0 && (GetLastError() != WSAEWOULDBLOCK)) {
			EditPrintf(hwndEdit, TEXT("error: connect to server: %s:%s failed (%d - %s)\n"), conn[i].ip, conn[i].port, GetLastError(), (char*)StrError());
			continue;
		}
		//EditPrintf(hwndEdit, TEXT("connect to server: %s:%s\n"), conn[i].ip, conn[i].port);

		/*** update status ***/
		conn[i].status = CONNECTING;
        ++activeConnNum;

		Write(curClient[curClientNo].fd, "<td>%s</td>\n", conn[i].ip);
	}
	Write(curClient[curClientNo].fd, "</tr>\n");

	/*** html table tbody ***/
    Write(curClient[curClientNo].fd, "<tr>\n");
    for(i = 0 ; i < connNum; ++i) {
        Write(curClient[curClientNo].fd, "<td valign='top' id='m%c'></td>\n", *(conn[i].no));
    }
    Write(curClient[curClientNo].fd, "</tr>\n");
    Write(curClient[curClientNo].fd, "</table>\n");

	return;
}

/*** the handler for handling the WM_CGI_NOTIFY event that driven by WSAAsyncSelect ***/
void CgiNotifyHandler(WPARAM wParam, LPARAM lParam) {
	int connNo = FindCurrentConn(wParam);
	if(connNo == -1) return;

	switch( WSAGETSELECTEVENT(lParam) ) {
	case FD_CONNECT:
		if(conn[connNo].status == CONNECTING) {

			/*** check non-blocking connect status ***/
			int error = 0, errorLen = sizeof(error);
			int res = 0;
			res = getsockopt(conn[connNo].connfd, SOL_SOCKET, SO_ERROR, (char*)&error, &errorLen);
			if(res < 0 || error != 0) {
				EditPrintf(hwndEdit, TEXT("error: fd: %d connection to %s:%s failed (return:%d, error: %d)(%d - %s)\n"),conn[connNo].connfd, conn[connNo].ip, conn[connNo].port, res, error, GetLastError(), StrError());
				DisconnectConn(connNo);
			}
			else {
				EditPrintf(hwndEdit, TEXT("fd: %d connect %s:%s successfully\n"),conn[connNo].connfd, conn[connNo].ip, conn[connNo].port);

				/*** non-blocking connect successfully, change status ***/
				conn[connNo].status = READING;
			}
		}
		break;

	case FD_READ:
		conn[connNo].readable = 1;
		while(conn[connNo].status == READING && conn[connNo].readable == 1) {
			ReadFromConn(connNo);
			if(conn[connNo].status == WRITING && conn[connNo].writable == 1) {
				WriteToConn(connNo);
			}
		}
		break;

	case FD_WRITE:
		conn[connNo].writable = 1;
		while(conn[connNo].status == WRITING && conn[connNo].writable == 1) {
			WriteToConn(connNo);
			if(conn[connNo].status == READING && conn[connNo].readable == 1) {
				ReadFromConn(connNo);
			}
		}
		break;

	case FD_CLOSE:
		DisconnectConn(connNo);
		break;
	}
	return;
}

/*** when receive notification, find the connection fd for cgi ***/
int FindCurrentConn(int fd) {
	int i = 0;
	for(; i < connNum; ++i) {
		if (fd == conn[i].connfd) {
			curConnNo = i;
			//EditPrintf(hwndEdit, TEXT("current connection for cgi at %d, fd is %d\n"), curConnNo, fd);
			break;
		}
	}
	if (i == connNum) {
		//EditPrintf(hwndEdit, TEXT("error: cannot find connection fd %d for cgi\n"), fd);
		return -1;
	}
	return i;
}

/*** disconnet the specific connection ***/
void DisconnectConn(int connNo) {
    conn[connNo].status = DONE;
    if(conn[connNo].connfd != -1) {
        closesocket(conn[connNo].connfd);
        conn[connNo].connfd = -1;
        if(conn[connNo].fp != NULL) fclose(conn[connNo].fp);
        conn[connNo].fp = NULL;
        --activeConnNum;
    }

	/*** cgi end, close client socket and reset all cgi environment parameters ***/
	if(activeConnNum == 0 && cgiUsing == 1) {
		FindCurrentClient(conn[connNo].clientfd);
		Disconnect();

		cgiUsing = 0;
		bzero(envParam, sizeof(envParam));
		bzero(param, sizeof(param));
		paramNum = 0;
		bzero(queryString, ENVPARAM_MAX_LEN);
		bzero(queryStringBak, ENVPARAM_MAX_LEN);
		queryStringLen = 0;
		connNum = 0;
		bzero(conn, sizeof(conn));
		activeConnNum = 0;
		curConnNo = -1;
	}
    return;
}

/*** read messages from the specific connection ***/
void ReadFromConn(int connNo) {
    int n = 0, res = 0;
	int* changeOption = &conn[connNo].changeOption;
    char c;
	char cgiBuf[BUF_MAX_SIZE];
    while( (res = recv(conn[connNo].connfd, &c, 1, 0)) > 0) {
		//EditPrintf(hwndEdit, TEXT("%c"), c);
		if(c == '\r') continue;

        /*** set changeOption flag ***/
        if(c == '%' && *changeOption == 0) {
            *changeOption = 1;
        }
		else if(c == ' ' && *changeOption == 1) {
			*changeOption = 2;
		}
        else if(c == '\n' && *changeOption != 2) {
            *changeOption = 0;
        }
        else if(*changeOption != 2) {
            *changeOption = -1;
        }

        /*** handle the read character ***/
        if(c == '\n') {
            cgiBuf[n] = '\0';
			//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"%s<br>\";</script>\n"), *conn[connNo].no, cgiBuf);
            Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"%s<br>\";</script>\n", *conn[connNo].no, cgiBuf);
            bzero(cgiBuf, BUF_MAX_SIZE);
            n = 0;
        }
        else if(n == BUF_MAX_SIZE - 3) {
            cgiBuf[n++] = c;
            cgiBuf[n] = '\0';
			//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"%s\";</script>\n"), *conn[connNo].no, cgiBuf);
			Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"%s\";</script>\n", *conn[connNo].no, cgiBuf);
            bzero(cgiBuf, BUF_MAX_SIZE);
            n = 0;
        }
        else {
            if(c == '\"') cgiBuf[n++] = '\\';
            cgiBuf[n++] = c;
            if(n == BUF_MAX_SIZE - 3) {
                cgiBuf[n++] = c;
                cgiBuf[n] = '\0';
				//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"%s\";</script>\n"), *conn[connNo].no, cgiBuf);
				Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"%s\";</script>\n", *conn[connNo].no, cgiBuf);
                bzero(cgiBuf, BUF_MAX_SIZE);
                n = 0;
            }
        }

        if(*changeOption == 2) break;
    }
    if(n > 0) {
        cgiBuf[n] = '\0';
		//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"%s\";</script>\n"), *conn[connNo].no, cgiBuf);
		Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"%s\";</script>\n", *conn[connNo].no, cgiBuf);
        bzero(cgiBuf, BUF_MAX_SIZE);
    }
    if(res == 0) {
        DisconnectConn(connNo);
        return;
    }
	else if(res == -1) {
		//EditPrintf(hwndEdit, TEXT("error: socket is not readable now (%d - %s)\n"), GetLastError(), StrError());
		conn[connNo].readable = 0;
	}

    /*** if read the '%' character, change the status to WRITING that can write a command line to the connection ***/
    if(*changeOption == 2) {
        EditPrintf(hwndEdit, TEXT("change fd: %d to WRITING status\n"), conn[connNo].connfd);
        conn[connNo].status = WRITING;
		*changeOption = 0;
    }
    else conn[connNo].status = READING;

    return;
}

/*** write a command line from the specific file to the specific connection  ***/
void WriteToConn(int connNo) {
    int j = 0, j2 = 0, lasterror = 0;
    char c;
	
	/*** buffer write to connection ***/
	char bufCgi[BUF_MAX_SIZE];

	/*** buffer write to client ***/
	char bufCgi2[BUF_MAX_SIZE];

    /*** the pipe type: <<EOF finite state machine (
     * 0:(ready for read the first '<'),
     * 1:<,
     * 2:<<,
     * 3:<<E,
     * 4:<<EO,
     * 5:<<EOF,
     * 6:<<EOF\n,
     * 7: (ready for read 'E')
     * 8:E,
     * 9:EO,
     * 10:EOF) ***/
	int* pipeEofFSM = &conn[connNo].pipeEofFSM;

    if(conn[connNo].fp != NULL) {
        while( (c = fgetc(conn[connNo].fp)) != EOF) {
            //EditPrintf(hwndEdit, TEXT("%c"), c);
			if(c == '\r') continue;

            if(c == '\n' && (*pipeEofFSM < 5 || *pipeEofFSM == 10)) {
                /*** write when read a '\n' character ***/

                *pipeEofFSM = 0;
                bufCgi2[j2] = '\0';
				//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n"), *conn[connNo].no, bufCgi2);
				Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n", *conn[connNo].no, bufCgi2);

                bufCgi[j++] = '\n';
                bufCgi[j] = '\0';
				//EditPrintf(hwndEdit, TEXT("%d bytes, %s"), j, bufCgi);
                Write(conn[connNo].connfd, bufCgi);
				lasterror = WSAGetLastError();
                bzero(bufCgi, BUF_MAX_SIZE);
                bzero(bufCgi2, BUF_MAX_SIZE);
				j = 0;
                j2 = 0;
                break;
            }
            else if(c == '\n' && *pipeEofFSM >= 5) {
                /*** write when read a '\n' character and then keep reading until read "EOF\n" ***/

                *pipeEofFSM = 7;
                bufCgi2[j2] = '\0';
				//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n"), *conn[connNo].no, bufCgi2);
				Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n", *conn[connNo].no, bufCgi2);

                bufCgi[j++] = '\n';
                bufCgi[j] = '\0';
				//EditPrintf(hwndEdit, TEXT("%d bytes, %s"), j, bufCgi);
                Write(conn[connNo].connfd, bufCgi);
				lasterror = WSAGetLastError();
                bzero(bufCgi, BUF_MAX_SIZE);
                bzero(bufCgi2, BUF_MAX_SIZE);
                j = 0;
                j2 = 0;

				if(lasterror == WSAEWOULDBLOCK) break;
                continue;
            }
            else if(c == '<' && (*pipeEofFSM == 0 || *pipeEofFSM == 1)) ++*pipeEofFSM;
            else if(c == 'E' && (*pipeEofFSM == 2 || *pipeEofFSM == 7)) ++*pipeEofFSM;
            else if(c == 'O' && (*pipeEofFSM == 3 || *pipeEofFSM == 8)) ++*pipeEofFSM;
            else if(c == 'F' && (*pipeEofFSM == 4 || *pipeEofFSM == 9)) ++*pipeEofFSM;
            else if(c == ' ' && *pipeEofFSM == 2) *pipeEofFSM = 2;
            else if(c == ' ' && *pipeEofFSM == -1) *pipeEofFSM = 0;
            else if(*pipeEofFSM >= 6) *pipeEofFSM = 6;
            else *pipeEofFSM = -1;
			//EditPrintf(hwndEdit, TEXT(" FSM: %d\n"), *pipeEofFSM);

            /*** handle the html encode ***/
            if(c == '<') {
                bufCgi2[j2++] = '&';
                bufCgi2[j2++] = 'l';
                bufCgi2[j2++] = 't';
                bufCgi2[j2++] = ';';
            }
            else if(c == '>') {
                bufCgi2[j2++] = '&';
                bufCgi2[j2++] = 'g';
                bufCgi2[j2++] = 't';
                bufCgi2[j2++] = ';';
            }
            else if(c == '\"') {
                bufCgi2[j2++] = '&';
                bufCgi2[j2++] = 'q';
                bufCgi2[j2++] = 'u';
                bufCgi2[j2++] = 'a';
                bufCgi2[j2++] = 't';
                bufCgi2[j2++] = ';';
            }
            else if(c == '&') {
                bufCgi2[j2++] = '&';
                bufCgi2[j2++] = 'a';
                bufCgi2[j2++] = 'm';
                bufCgi2[j2++] = 'p';
                bufCgi2[j2++] = ';';
            }
            else if(c == ' ') {
                bufCgi2[j2++] = '&';
                bufCgi2[j2++] = 'n';
                bufCgi2[j2++] = 'b';
                bufCgi2[j2++] = 's';
                bufCgi2[j2++] = 'p';
                bufCgi2[j2++] = ';';
            }
            else bufCgi2[j2++] = c;

            bufCgi[j++] = c;

            /*** write when buffer is full and then keep read ***/
            if(j2 >= BUF_MAX_SIZE - 6) {
                bufCgi2[j2] = '\0';
				//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"<b>%s</b>\";</script>\n"), *conn[connNo].no, bufCgi2);
				Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"<b>%s</b>\";</script>\n", *conn[connNo].no, bufCgi2);

                buffer[j] = '\0';
				//EditPrintf(hwndEdit, TEXT("%d bytes, %s"), j, bufCgi);
                Write(conn[connNo].connfd, bufCgi);
				lasterror = WSAGetLastError();
                j = 0;
                j2 = 0;
                bzero(bufCgi, BUF_MAX_SIZE);
                bzero(bufCgi2, BUF_MAX_SIZE);
				if(lasterror == WSAEWOULDBLOCK) break;
            }
        }

		/*** check whether buffer still has data, write if so ***/
		if(j > 0 && j2 > 0) {
			*pipeEofFSM = 0;
			bufCgi2[j2] = '\0';
			//EditPrintf(hwndEdit, TEXT("<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n"), *conn[connNo].no, bufCgi2);
			Write(conn[connNo].clientfd, "<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n", *conn[connNo].no, bufCgi2);

			bufCgi[j++] = '\n';
			bufCgi[j] = '\0';
			EditPrintf(hwndEdit, TEXT("%d bytes, %s"), j, bufCgi);
			Write(conn[connNo].connfd, bufCgi);
			lasterror = WSAGetLastError();
		}

		/*** check whether system is writable after last write ***/
		if(lasterror == WSAEWOULDBLOCK) {
			conn[connNo].writable = 0;
			//EditPrintf(hwndEdit, TEXT("error: write buffer is full (%d - %s)\n"), lasterror, StrError());
		}

        /*** close the batch file ***/
        if(c == EOF) {
            fclose(conn[connNo].fp);
            conn[connNo].fp = NULL;
            //DisconnectConn(connNo);
            //EditPrintf(hwndEdit, TEXT("end of file %s\n"), conn[connNo].batchFile);
            //return;
        }

        /*** after write to connection, change the status to READING that can wait for response from connection  ***/
        //EditPrintf(hwndEdit, TEXT("change fd: %d to READING status\n"), conn[connNo].connfd);
        conn[connNo].status = READING;
    }
    else {
		//EditPrintf(hwndEdit, TEXT("error: nothing to write\n"));
        DisconnectConn(connNo);
    }

    return;
}

void ListAllConnVar() {
    int i = 0;
    for(; i < connNum; ++i) {
        EditPrintf(hwndEdit, TEXT("server %d (fd:%d)["), i + 1, conn[i].connfd);
        EditPrintf(hwndEdit, TEXT("ip(h%c): %s, "), *(conn[i].no), conn[i].ip);
        EditPrintf(hwndEdit, TEXT("port(p%c): %s, "), *(conn[i].no), conn[i].port);
        EditPrintf(hwndEdit, TEXT("batch file(f%c): %s.]\n"), *(conn[i].no), conn[i].batchFile);
    }
    return;
}
