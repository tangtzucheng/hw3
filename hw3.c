#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <time.h>
#include <arpa/inet.h>

#define IPv4 0x0800
#define IPv6 0x086dd
#define TCP 6
#define UDP 17

typedef struct eheader {
	u_char dst_mac[6];
	u_char src_mac[6];
	u_short etype;
}eheader;
eheader *ethernet;

typedef struct ip_header {
	int version:4;
	int header_len:4;
	u_char tos:8;
	int total_len:16;
	int ident:16;
	int flags:16;
	u_char ttl:8;
	u_char protocol:8;
	int checksum:16;
	u_char sourceIP[4];
	u_char destIP[4];
}ip_header;
ip_header *ip;

typedef struct ip6_header{
	unsigned int version:4;
	unsigned int traffic_class:8;
	unsigned int flow_label:20;
	uint16_t payload_len;
	uint8_t next_header;
	uint8_t hop_limit;
	uint16_t sourceIP[8];
	uint16_t destIP[8];
}ip6_header;
ip6_header *ip6;

typedef struct tcp_header {
	u_short sport;
	u_short dport;
	u_int seq;
	u_int ack;
	u_char head_len;
	u_char flags;
	u_short wind_size;
	u_short check_sum;
	u_short urg_ptr;
}tcp_header;
tcp_header *tcp;

typedef struct udp_header {
	u_short sport;
	u_short dport;
	u_short tot_len;
	u_short check_sum;
}udp_header;
udp_header *udp;

void packet_handler(u_char *arg, const struct pcap_pkthdr *pkt_header, const u_char *pkt_data) {
	static int count = 0;
	count++;

	printf("No.%d\n\n",count);
	printf("Time: %s",ctime((const time_t*)&pkt_header->ts.tv_sec));
	printf("Len: %d\n", pkt_header->len);
	u_int eth_len=sizeof(struct eheader);
	u_int ip_len=sizeof(struct ip_header);
	u_int ip6_len=sizeof(struct ip6_header);

	ethernet=(eheader *)pkt_data;
	if(ntohs(ethernet->etype) == IPv4) {
		printf("\nHeader:\n");
		ip=(ip_header*)(pkt_data+eth_len);
		printf("SRC ip: %d.%d.%d.%d\n",ip->sourceIP[0],ip->sourceIP[1],ip->sourceIP[2],ip->sourceIP[3]);
		printf("DST ip: %d.%d.%d.%d\n",ip->destIP[0],ip->destIP[1],ip->destIP[2],ip->destIP[3]);
		if(ip->protocol == TCP) {
			tcp=(tcp_header*)(pkt_data+eth_len+ip_len);
			printf("\nprotocol: TCP\n");
			printf("SRC port: %u\n",htons(tcp->sport));
			printf("DST port: %u\n",htons(tcp->dport));
		}
		else if(ip->protocol == UDP) {
			udp=(udp_header*)(pkt_data+eth_len+ip_len);
			printf("\nprotocol: UDP\n");
			printf("SRC port: %u\n",htons(udp->sport));
			printf("DST port: %u\n",htons(udp->dport));
		 }
		else {
			printf("protocol: xxxxxx\n");
		}
	}
	else if(ntohs(ethernet->etype) == IPv6) {
        printf("\nHeader:\n");
		ip6=(ip6_header*)(pkt_data+eth_len);
		char str[INET6_ADDRSTRLEN];
		printf("SRC ip6:%s\n",inet_ntop(AF_INET6,ip6->sourceIP,str,sizeof(str)));
		printf("DST ip6:%s\n",inet_ntop(AF_INET6,ip6->destIP,str,sizeof(str)));
        if(ip6->next_header == TCP) {
			tcp=(tcp_header*)(pkt_data+eth_len+ip6_len);
			printf("\nTCP protocol:\n");
			printf("source port: %u\n",htons(tcp->sport));
			printf("dest   port: %u\n",htons(tcp->dport));
        }
        else if(ip6->next_header == UDP) {
			udp=(udp_header*)(pkt_data+eth_len+ip6_len);
			printf("\nUDP protocol:\n");
			printf("SRC port: %u\n",htons(udp->sport));
			printf("DST port: %u\n",htons(udp->dport));
            }
        else {
			printf("protocol:  xxxxxx\n");
        }
	}
	printf("\n\n");
}

int main(int argc,char *argv[]) {
    char *condition = "any";
    bpf_u_int32 net;
    bpf_u_int32 mask;

    pcap_t *handle;
    struct bpf_program filter;
    char errbuf[PCAP_ERRBUF_SIZE];

    if (argc == 4) {
        condition = argv[3];
    }
    printf("File: %s\nFilter: %s\n\n", argv[2], condition);

	handle = pcap_open_offline(argv[2], errbuf);
    pcap_compile(handle, &filter, argv[3], 0, net);
    pcap_setfilter(handle, &filter);
    pcap_loop(handle,-1, packet_handler, NULL);
    pcap_close(handle);

    return 0;
}
