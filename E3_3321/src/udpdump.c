#define HAVE_REMOTE
 #include <stdlib.h>
#include <pcap.h>
#include <Packet32.h>
#include<time.h>
#include <ntddndis.h>
#include<Windows.h>
#pragma comment(lib, "Packet")
#pragma comment(lib, "wpcap")
#pragma comment(lib, "WS2_32")

/* 4 bytes IP address */
typedef struct ip_address
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header {
	u_char saddr[4]; // Source address
	u_char daddr[4]; // Destination address
} ip_header;

/* mac header*/
typedef struct mac_header {
	u_char dest_addr[6];
	u_char src_addr[6];
	u_char type[2];
} mac_header;

pcap_t* adhandle;
int first_time;

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
static int length = 0;
int main()
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;
	char errbuf[PCAP_ERRBUF_SIZE];
	u_int netmask;
	char packet_filter[] = "ip and udp";
	struct bpf_program fcode;
	
	/* Retrieve the device list */
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs,errbuf) == -1) {
		fprintf(stderr, "Error in pcap_findalldevs: %s\n",errbuf);
		exit(1);
	}
	/* Print the list */
	for (d = alldevs; d; d = d->next) {
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0) {
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
			return -1;
	}
	printf("Enter the interface number (1-%d):", i);
	scanf_s("%d", &inum);

	/* Check if the user specified a valid adapter */
	if (inum < 1 || inum > i) {
		printf("\nInterface number out of range.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* Jump to the selected adapter */
	for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);
	
	/* Open the adapter */
	if ((adhandle = pcap_open(d->name, 65536,
		PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)) == NULL) {
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n");
			pcap_freealldevs(alldevs);
		return -1;
	}
	
	/* Check the link layer. We support only Ethernet for simplicity. */
	if(pcap_datalink(adhandle) != DLT_EN10MB)
	{
		fprintf(stderr,"\nThis program works only on Ethernet networks.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	if (pcap_datalink(adhandle) != DLT_EN10MB) {
		fprintf(stderr,"\nThis program works only on Ethernet networks.\n");
			pcap_freealldevs(alldevs);
		return -1;
	}
	if (d->addresses != NULL)
		netmask = ((struct sockaddr_in*)(d->addresses -> netmask))->sin_addr.S_un.S_addr;
	else
		netmask = 0xffffff;

	//compile the filter
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask)
		< 0) {
		fprintf(stderr,	"\nUnable to compile the packet filter. Check the syntax.\n");
			pcap_freealldevs(alldevs);
		return -1;
	}
	//set the filter
	if (pcap_setfilter(adhandle, &fcode) < 0) {
		fprintf(stderr,
			"\nError setting the filter.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	printf("\nlistening on %s...\n", d->description);
	
	/* At this point, we don't need any more the device list.Free it */
	pcap_freealldevs(alldevs);

	time_t timep;
	struct tm* p;
	time(&timep);
	p = gmtime(&timep);
	first_time= (p->tm_hour + 8) * 3600 + (p->tm_min) * 60 + p->tm_sec;
	/* start the capture */
	pcap_loop(adhandle, 0, packet_handler, NULL);
	printf("一分钟内接收到的数据量为：%d", length);
	FILE* fp;
	fp = fopen("D:\\temp\\test.txt", "w"); 
	fprintf(fp, "一分钟内接收到的数据量为：%d", length); 
	fclose(fp);
	return 0;
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	FILE* fp;
	fp = fopen("D:\\temp\\record.txt", "a+"); //由于反斜杠\是控制字符，所以必须再加一个反斜杠
	int last_time;
	struct tm *ltime;
	char timestr[16];
	ip_header *ih;
	mac_header* mh;
	u_int ip_len;
	u_short sport,dport;
	time_t local_tv_sec;

	/*
	 * unused parameter
	 */
	(VOID)(param);
	/* convert the timestamp to readable format */
	time_t timep;
	struct tm* p;
	time(&timep);
	p = gmtime(&timep);
	printf("%d-%d-%d ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);
	last_time = (p->tm_hour + 8) * 3600 + (p->tm_min) * 60 + p->tm_sec;
	
	local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);
	printf("%s,", timestr);

	mh = (mac_header*)pkt_data;
	ih = (ip_header*)(pkt_data + sizeof(mac_header));

	/*print source MAC*/
	for (int i = 0; i < 6; i++) {
		if (i != 5)
			printf("%02X-", mh->src_addr[i]);
		else printf("%02X,", mh->src_addr[i]);
	}

	/*print source ip*/
	for (int i = 0; i < 4; i++) {
		if(i!=3)
		printf("%d.", ih->saddr[i]);
		else
			printf("%d,", ih->saddr[i]);
	}

    /*print destination MAC*/
	for (int i = 0; i < 6; i++) {
		if(i!=5)
		printf("%02X-", mh->dest_addr[i]);
		else printf("%02X,", mh->dest_addr[i]);
	}

;   /*print destination ip*/
	for (int i = 0; i < 4; i++) {
		if(i!=3)
		printf("%d.", ih->daddr[i]);
		else 
			printf("%d,", ih->daddr[i]);
	}

	/*print length*/
	printf("%d ", header->len);
	printf("\n");
	length = length + header->len;
	fprintf(fp, "%d-%d-%d %s,%02X-%02X-%02X-%02X-%02X-%02X,%d.%d.%d.%d,%02X-%02X-%02X-%02X-%02X-%02X,%d.%d.%d.%d,%d\n", 
		(1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday,timestr,mh->src_addr[0], mh->src_addr[1], mh->src_addr[2],
		 mh->src_addr[3], mh->src_addr[4], mh->src_addr[5], ih->saddr[0], ih->saddr[1], ih->saddr[2], ih->saddr[3],
		 mh->dest_addr[0], mh->dest_addr[1], mh->dest_addr[2], mh->dest_addr[3], mh->dest_addr[4], mh->dest_addr[5], 
		 ih->daddr[0],ih->daddr[1], ih->daddr[2],ih->daddr[3],header->len);
	if (last_time - first_time >= 60)
	{
		fclose(fp);
		pcap_breakloop(adhandle);
	}
}
