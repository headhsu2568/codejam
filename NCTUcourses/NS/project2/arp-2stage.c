#include <stdio.h>
#include <memory.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>


/* Ethernet addresses are 6 bytes */
#define ETHER_LEN 6
/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

#define IP_ALEN 4

#define ARPOP_REPLY 2
#define ARPHDR_ETHER 1
#define ETH_P_ARP 0x0806
#define ETH_P_IP 0x0800
#define LEN 42
/* Ethernet header */
struct ether_header {
        u_char  ether_dhost[ETHER_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_LEN];    /* source host address */
        u_short ether_type;                     /* IP? ARP? RARP? etc */
};
// ARP Header Struktur
struct arp_header
{
    u_short hw_type; // hardware type
    u_short proto_type; // protocol type
    char ha_len; // hardware address length
    char pa_len; // protocol address length
    u_short opcode; // arp opcode
    unsigned char source_add[ETHER_LEN]; // source mac
    unsigned char source_ip[IP_ALEN]; // source ip
    unsigned char dest_add[ETHER_LEN]; // destination mac
    unsigned char dest_ip[IP_ALEN]; // destination ip
};

/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void print_hex_ascii_line(const u_char *payload, int len, int offset)
{
	int i;
	int gap;
	const u_char *ch;
	/* offset */
	printf("%05d   ", offset);
	
	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");
	
	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");
	
	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");

return;
}

/*
 * print packet payload data (avoid printing binary data)
 */
void print_payload(const u_char *payload, int len)
{

	int len_rem = len;
	int line_width = 16;			/* number of bytes per line */
	int line_len;
	int offset = 0;					/* zero-based offset counter */
	const u_char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	/* data spans multiple lines */
	for ( ;; ) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}
	return;
}

void get_mac(const char* dev,char* smac)
{
	memset(smac,ETHER_LEN,'\0');
	struct ifreq iface;

		// Read MAC Address
    		strcpy(iface.ifr_name, dev);
		int r_sock;
		if((r_sock = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ARP))) < 0)
		{
			perror("socket");
			exit(1);
		}
		if((ioctl(r_sock, SIOCGIFHWADDR, &iface)) < 0)
		{
			perror("ioctl SIOCGIFHWADDR");
			exit(1);
		}
		else
		{
			memcpy(smac,iface.ifr_hwaddr.sa_data,ETHER_LEN);
		}
		close(r_sock);
}

//change ip xxx.xxx -> hostent
void resolve(char *host,char *result)
{
	struct hostent *hent;
      
	if ((hent=gethostbyname(host))==NULL) 
	{
		printf("Unable to resolve host %s\n",host);
		exit(1);
	}
	memcpy(result,hent->h_addr,4);
}
void send_packet(char* dev,char* packet)
{
	struct sockaddr addr;
	const len=sizeof(struct arp_header) + sizeof(struct ether_header);
	int sock=0;
	// Raw Socket to write
	if((sock = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ARP))) < 0)
	{
		perror("socket");
		exit(1);
	}

	strncpy(addr.sa_data, dev, sizeof(addr.sa_data));
    //fill src code here : use sendto function to send packet out
	close(sock);
}
void build(char* dev,char* smac, char* sip, char* dmac, char* dip, unsigned char* arppacket)
{
	const len=sizeof(struct arp_header) + sizeof(struct ether_header);
	struct ether_header *spoof_eth = (struct ether_header *)arppacket;
	struct arp_header *spoof_arp = (struct arp_header *)(arppacket + sizeof(struct ether_header));

	//Initialize 
	memset(arppacket,len,'\0');
    //fill src code here
	//Make Fake Ethernet Header
	       //set Destination MAC
	       //set Source MAC
           //set Packet type

	//Make Fake ARP Header
	       //set Hardware address type
	       //set Protocol address type
	       //set Hardware address length
           //set Protocol address length
	       //set ARP operation type
	       //set Sender MAC
	       //set Source IP
	       //set Target MAC
	       //set Target IP
		
}

// change mac to 16
void translate_mac(char* src,char* dst)
{
	int len=strlen(src);
	int i,j;
	//format 11:22:33:44:55:66 
	for (i=j=0;j<len;j+=3,i++)
	{
		dst[i]=strtol(src+j,NULL,16);
	}
}

int main(int argc, char *argv[])
{
	char *dev = NULL;			/* capture device name */
	char dmac[ETHER_LEN]={0},smac[ETHER_LEN]={0};
	char sip[IP_ALEN]={0},dip[IP_ALEN]={0};
	/* check for capture device name on command-line */
	if (argc == 6) {
		unsigned char packet[LEN];
		dev = argv[1];
		translate_mac(argv[2],smac);
		resolve(argv[3],sip);
		translate_mac(argv[4],dmac);
		resolve(argv[5],dip);
		build(dev,smac,sip,dmac,dip,packet);
		
		
		send_packet(dev,packet);		
		
	}
	else 
	{
		printf("Usage: $CMD <interface> <src_MAC> <src_IP> <dst_MAC> <dst_IP> \n");
	}
	return 0;
}
