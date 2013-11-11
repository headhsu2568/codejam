/*

   NOTE 

   before write string to server, add \n or \r\n in the string end
   Please check the end of the string for \r and \n to avoid accident

   USAGE

   ./client server-ip server-port student-id

   COMPILE

   gcc -o client client.c des.c

   Functions Help

 * DES Example
 * -----------
 *     unsigned char key[8];
 *     unsigned char plaintext[8];
 *     unsigned char ciphertext[8];
 *     unsigned char recoverd[8];
 *     gl_des_ctx context;
 *
 *     // Fill 'key' and 'plaintext' with some data
 *     ....
 *
 *     // Set up the DES encryption context
 *     gl_des_setkey(&context, key);
 *
 *     // Encrypt the plaintext
 *     des_ecb_encrypt(&context, plaintext, ciphertext);
 *
 *     // To recover the orginal plaintext from ciphertext use:
 *     des_ecb_decrypt(&context, ciphertext, recoverd);


*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>
#include <netdb.h>
#include <netinet/in.h>
#include "des.h"
#define length 128
int readline(int socket, char * buf, int len);  		//read string 

void usage(char *prog)
{
	fprintf(stderr, "usage: %s server-ip server-port student-id\n", prog);
	exit(1);
}


int main(int argc, char *argv[])
{
	int s;				//server socket
	int port;
	struct sockaddr_in server;
	char *ptr;
	struct hostent *host;

	char buf[length];			//get server message
	int len;
	unsigned char deskey[80]={0};		//the DES key
	unsigned char plaintext[80]={0};	
	unsigned char ciphertext[80]={0};	
	unsigned char recoverd[80]={0};		//get decrypt ciphertext
	gl_des_ctx context;

	if (argc != 4 )
		usage(argv[0]);

	port = strtol(argv[2], &ptr, 10);
	if(ptr == argv[2] || *ptr != '\0')
		usage(argv[0]);

	//set des key
	strncpy(deskey,argv[3],strlen(argv[3]));

	if((host = gethostbyname(argv[1])) == NULL)
	{
		fprintf(stderr,"gethostbyname(): %s\n", hstrerror(h_errno));
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr = *((struct in_addr *)host->h_addr);

	if((s = socket(PF_INET,SOCK_STREAM, 0)) <0)
	{
		fprintf(stderr,"socket(): %s\n",strerror(errno));
		exit(1);
	}

	if(connect(s, (struct sockaddr *)&server, sizeof(server))<0)
	{
		fprintf(stderr,"connect(): %s\n",strerror(errno));
		exit(1);
	}


	//read prompt
	if(readline(s, buf, length)<0)
	{
		fprintf(stderr, "read error\n");
		exit(1);
	}


	len = sprintf(buf, "%s\n", argv[3]);
	if(write(s, buf, len) != len)
	{
		fprintf(stderr, "write(): %s\n", strerror(errno));
		exit(1);
	}


	/* 1. Server send a plain to client */
	if((len = readline(s, buf, length))<0)
	{
		fprintf(stderr, "read error\n");
		exit(1);
	}

	strncpy(plaintext,buf,strlen(buf)-1);

	/******** 2. Client encrypt string *******/
	//step 1. use gl_des_setkey to define DES encryption/decryption key
	gl_des_setkey(&context, deskey);
	//step 2. use gl_des_ecb_encrypt to encrypt plaintext
	gl_des_ecb_encrypt(&context, plaintext, ciphertext);
	//step 3. convert hex value to ascii value, 
	//		ex: one byte hex value "ab" convert to two byte hex value "61 62"
	//step 4. save convert result to my_buf 
	unsigned char tmp[2];
	unsigned char my_tmp[3];
	unsigned char my_buf[160]={0};
	int j=0;
	int n=0;
	for(;j<strlen(ciphertext);){
		tmp[0]=ciphertext[j++];
		tmp[1]='\0';
		sprintf(my_tmp,"%.2x",tmp[0]);
		my_buf[n++]=my_tmp[0];
		my_buf[n++]=my_tmp[1];
	}
	/***********End encrypt string***********/


	/* 3. Client send reply ciphertext to server*/
	strcat(my_buf,"\r\n");
	if (write(s,my_buf,strlen(my_buf)) != strlen(my_buf)) {
		fprintf(stderr, "write(): %s\n",strerror(errno));
		exit(1);
	}

	/* 4. Server send response */
	if((len = readline(s, buf, length))<0)
	{
		fprintf(stderr, "read error\n");
		exit(1);
	}

	fprintf(stderr,"About the Encryption Part, the Answer is: %s\n", buf);
	/* 5. Server send a ciphertext to client */
	memset(buf,0,length);
	if((len = readline(s, buf, length))<0)
	{
		fprintf(stderr, "read error\n");
		exit(1);
	}
	fprintf(stderr," CipherText is %s \n",buf);
	/********* 6. Client decrypt string *********/
	//step 1. convert ascii value to hex value
	int i=0;
	int k=0;
	unsigned char tmp2;
	unsigned char my_tmp2[3];
	for(;i<strlen(buf);){
		int tmp_ascii=0;
		my_tmp2[0]=buf[i++];
		my_tmp2[1]=buf[i++];
		my_tmp2[2]='\0';
		tmp_ascii=strtol(my_tmp2,NULL,16);
		sprintf(&tmp2,"%c",tmp_ascii);
		ciphertext[k++]=tmp2;
	}
	//step 2. use gl_des_ecb_decrypt to decrypt ciphertext
	gl_des_ecb_decrypt(&context, ciphertext, recoverd);
	//step 3. save plaintext in recoverd
	/***********End decrypt string **************/


	/* 7. Client send reply plaintext to server*/
	strcat(recoverd,"\r\n");
	if(write(s,recoverd,strlen(recoverd)) != strlen(recoverd))
	{
		fprintf(stderr, "write(): %s\n",strerror(errno));
		exit(1);
	}

	/* 8. Server send response */
	if((len = readline(s, buf, length ))<0)
	{
		fprintf(stderr, "read error\n");
		exit(1);
	}

	printf("Server says: %s\n", buf);

	close(s);
	exit(0);
}

int readline(int socket, char * buf, int len)
{
	static char * bp;
	static int cnt = 0;
	static char b[1500];

	char *bufx =buf;
	char c;
	char srfound = 0;

	while(--len>0)
	{
		if(--cnt<=0)
		{
			cnt = read(socket,b,sizeof(b));
			//printf("recv = %d\n",cnt);
			if(cnt < 0)
			{
				if(errno == EINTR)
				{
					len++;
					continue;
				}
				return -1;
			}
			if(cnt == 0)
				return 0;
			bp  = b;

		}

		c = *bp++;
		*buf++ = c;

		if(c == '\r')
			srfound = 1;
		else if (c != '\n')
			srfound = 0;

		if(c == '\n')
		{
			if(srfound)
				buf--;

			*buf = '\0';
			return buf - bufx -1 ;
		}
	}
	return -1;
}

