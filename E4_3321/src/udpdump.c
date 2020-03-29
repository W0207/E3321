#include <stdio.h>
#include <stdlib.h>
#include<stdbool.h>
#define HAVE_REMOTE
#include "pcap.h"
#include <ws2tcpip.h>
#define LINE_LEN 16

u_char user[20];//�û���
u_char pass[20];//����

/* 4�ֽڵ�IP��ַ */
typedef struct ip_address {
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;

/* IPv4 header*/
typedef struct ip_header {
    u_char ver_ihl; // Version (4 bits) +Internet header length (4 bits)
    u_char tos; // Type of service
    u_short tlen; // Total length
    u_short identification; // Identification
    u_short flags_fo; // Flags (3 bits) + Fragmentoffset (13 bits)
    u_char ttl; // Time to live
    u_char proto; // Protocol
    u_short crc; // Header checksum
    ip_address  saddr;      // Դ��ַ(Source address)
    ip_address  daddr;      // Ŀ�ĵ�ַ(Destination address)
}ip_header;

/* mac header*/
typedef struct mac_header {
    u_char dest_addr[6];
    u_char src_addr[6];
    u_char type[2];
} mac_header;
/* �ص�����ԭ�� */
void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data);

main()
{
    pcap_if_t* alldevs;
    pcap_if_t* d;
    int inum;
    int i = 0;
    pcap_t* adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    u_int netmask;
    char packet_filter[] = "tcp dst port ftp";
    struct bpf_program fcode;

    /* ����豸�б� */
    if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }

    /* ��ӡ�б� */
    for (d = alldevs; d; d = d->next)
    {
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }

    if (i == 0)
    {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return -1;
    }

    printf("Enter the interface number (1-%d):", i);
    scanf("%d", &inum);

    if (inum < 1 || inum > i)
    {
        printf("\nInterface number out of range.\n");
        /* �ͷ��豸�б� */
        pcap_freealldevs(alldevs);
        return -1;
    }

    /* ��ת����ѡ�豸 */
    for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

    /* �������� */
    if ((adhandle = pcap_open(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf  )) == NULL)
    {
        fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
        /* �ͷ��豸�б� */
        pcap_freealldevs(alldevs);
        return -1;
    }

    /* ���������·�㣬Ϊ�˼򵥣�����ֻ������̫�� */
    if (pcap_datalink(adhandle) != DLT_EN10MB)
    {
        fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
        /* �ͷ��豸�б� */
        pcap_freealldevs(alldevs);
        return -1;
    }

    if (d->addresses != NULL)
        /* ��ýӿڵ�һ����ַ������ */
        netmask = ((struct sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
    else
        /* ����ӿ�û�е�ַ����ô���Ǽ���һ��C������� */
        netmask = 0xffffff;

    //���������
    if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) < 0)
    {
        fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
        /* �ͷ��豸�б� */
        pcap_freealldevs(alldevs);
        return -1;
    }

    //���ù�����
    if (pcap_setfilter(adhandle, &fcode) < 0)
    {
        fprintf(stderr, "\nError setting the filter.\n");
        /* �ͷ��豸�б� */
        pcap_freealldevs(alldevs);
        return -1;
    }

    printf("\nlistening on %s...\n", d->description);

    /* �ͷ��豸�б� */
    pcap_freealldevs(alldevs);

    /* ��ʼ��׽ */
    pcap_loop(adhandle, 0, packet_handler, NULL);

    return 0;
}

void output(ip_header* ih, mac_header* mh, const struct pcap_pkthdr* header, char user[], char pass[], bool isSucceed)
{
    if (user[0] == '\0')
        return;
    
    char timestr[46];
    struct tm* ltime;
    time_t local_tv_sec;

    /*��ʱ���ת���ɿ�ʶ��ĸ�ʽ */
    local_tv_sec = header->ts.tv_sec;
    ltime = localtime(&local_tv_sec);
    strftime(timestr, sizeof timestr, "%Y-%m-%d %H:%M:%S", ltime);

    /*
        ���������̨
    */
    printf("%s,", timestr);//ʱ��
 
    printf("%02X-%02X-%02X-%02X-%02X-%02X,",
        mh->src_addr[0],
        mh->src_addr[1],
        mh->src_addr[2],
        mh->src_addr[3],
        mh->src_addr[4],
        mh->src_addr[5]);


    printf("%d.%d.%d.%d,",
        ih->saddr.byte1,
        ih->saddr.byte2,
        ih->saddr.byte3,
        ih->saddr.byte4);

    printf("%02X-%02X-%02X-%02X-%02X-%02X,",
        mh->dest_addr[0],
        mh->dest_addr[1],
        mh->dest_addr[2],
        mh->dest_addr[3],
        mh->dest_addr[4],
        mh->dest_addr[5]);


    printf("%d.%d.%d.%d,",
        ih->daddr.byte1,
        ih->daddr.byte2,
        ih->daddr.byte3,
        ih->daddr.byte4);


    printf("%s,%s,", user, pass);//�˺�����

    if (isSucceed) {
        printf("SUCCEED\n");
    }
    else {
        printf("FAILED\n");
    }


    /*
        ������ļ�
    */
    FILE* fp = fopen("D:\\record.csv", "a+");
    fprintf(fp, "%s,", timestr);//ʱ��


    fprintf(fp, "%02X-%02X-%02X-%02X-%02X-%02X,",
        mh->src_addr[0],
        mh->src_addr[1],
        mh->src_addr[2],
        mh->src_addr[3],
        mh->src_addr[4],
        mh->src_addr[5]);
    fprintf(fp, "%d.%d.%d.%d,",
        ih->saddr.byte1,
        ih->saddr.byte2,
        ih->saddr.byte3,
        ih->saddr.byte4);

    fprintf(fp, "%02X-%02X-%02X-%02X-%02X-%02X,",
        mh->dest_addr[0],
        mh->dest_addr[1],
        mh->dest_addr[2],
        mh->dest_addr[3],
        mh->dest_addr[4],
        mh->dest_addr[5]);
    fprintf(fp, "%d.%d.%d.%d,",
        ih->daddr.byte1,
        ih->daddr.byte2,
        ih->daddr.byte3,
        ih->daddr.byte4);


    fprintf(fp, "%s,%s,", user, pass);//�˺�����

    if (isSucceed) {
        fprintf(fp, "SUCCEED\n");
    }
    else {
        fprintf(fp, "FAILED\n");
    }
    fclose(fp);

    user[0] = '\0';
}

void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data)
{
    ip_header* ih;
    mac_header* mh;
    u_int i = 0;
    int head = 54;
    int length = sizeof(mac_header) + sizeof(ip_header);
    mh = (mac_header*)pkt_data;
    ih = (ip_header*)(pkt_data + 14); //length ofethernet header

    int name_point = 0;
    int pass_point = 0;
    int tmp;
    if ((char)pkt_data[head] == 'U')
    {
       name_point = head + 5;//'u' 's' 'e' 'r' ' '��5���ֽ�,��ת���û�����һ���ֽ�
       //���س�0x0d��ascii 13�����У�ascii 10��Ϊֹ��ǰ����������û���
       int j = 0;
       while (!(*(pkt_data + name_point) == 13 && *(pkt_data + name_point + 1) == 10))
       {
            user[j] = *(pkt_data + name_point);//�洢�˺�
            j++;
            ++name_point;
       }
    }
    if ((char)pkt_data[head] == 'P') 
    {
       pass_point = head + 5;//'P' 'A' 'S' 'S' ' '��5���ֽ�,��ת�������һ���ֽ�
       tmp = pass_point;
       //���س�0x0d��ascii 13�����У�ascii 10��Ϊֹ��ǰ�������������
       int k = 0;
       while (!(*(pkt_data + pass_point) == 13 && *(pkt_data + pass_point + 1) == 10))
       {
       pass[k] = *(pkt_data + pass_point);//�洢����
       k++;
       ++pass_point;
       }

       for (;; tmp++)
       {
           if ((char)pkt_data[tmp] == '5' && *(pkt_data + tmp + 1) == '3' && *(pkt_data + tmp + 2) == '0')
           {
               output(ih, mh, header, (char*)user, (char*)pass, false);
               break;
           }
           else
           {
               output(ih, mh, header, (char*)user, (char*)pass, true);
               break;
           }
       }
    }
}
