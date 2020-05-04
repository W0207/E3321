#include<iostream>
#include<WinSock2.h>
#include<winsock.h>
#include<Windows.h>
#include<string>
#include<cstring>
#include <fstream>
#include <io.h>
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 1024
#define SERVER_ID "127.0.0.1"
#define PATH_LENGTH 20
using namespace std;
char sendBuff[BUF_SIZE];
char recvBuff[BUF_SIZE];
char fileName[PATH_LENGTH];

int main() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "Initialization failed." << endl;
		return -1;
	}
	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == -1) {
		cout << "Socket failed." << endl;
		return -1;
	}
	sockaddr_in sadr;
	sadr.sin_family = AF_INET;
	sadr.sin_port = htons(5000);
	sadr.sin_addr.S_un.S_addr = inet_addr(SERVER_ID);
	int nAddrlen = sizeof(sadr);
	while (true) {
		cout << "---------------------SENDING...---------------------" << endl;
		cout << "Please input the filename: " << endl;
		cin >> fileName;
		FILE* fp;
		if (!(fp = fopen(fileName, "rb"))) {
			cout << "Fail to open file." << endl;
			continue;
		}
		//先传送文件名
		sendto(client, fileName, strlen(fileName), 0, (sockaddr*)&sadr, sizeof(sadr));
		int length;
		int ret;
		while ((length = fread(sendBuff, 1, BUF_SIZE, fp)) > 0) {
			ret = sendto(client, sendBuff, length, 0, (sockaddr*)&sadr, sizeof(sadr));
			if (!ret) {
				cout << "An error occurred while sending." << endl;
				return -1;
			}
			ret = recvfrom(client, recvBuff, BUF_SIZE, 0, (sockaddr*)&sadr, &nAddrlen);
			if (!ret) {
				cout << "Fail to receive." << endl;
				return -1;
			}
			else {
				if (strcmp(recvBuff, "success")) {
					cout << "Fail to receive." << endl;
					return -1;
				}
			}
		}
		//传送文件发送结束信息
		char end_flag[10] = "end";
		ret = sendto(client, end_flag, length, 0, (sockaddr*)&sadr, sizeof(sadr));
		fclose(fp);
	}
	closesocket(client);
	WSACleanup();
	return 0;
}
