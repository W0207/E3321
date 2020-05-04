#include<iostream>
#include<WinSock2.h>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 1024
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
	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == -1) {
		cout << "Socket failed." << endl;
		return -1;
	}
	sockaddr_in my_addr, remote_addr;
	int nAddrlen = sizeof(remote_addr);
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(5000);
	my_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (::bind(server, (sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR) {
		cout << "Bind error!" << endl;
		return -1;
	}
	while (true) {
		cout << "---------------------RECEIVING...---------------------" << endl;
		//�����ļ���
		int ret = recvfrom(server, fileName, BUF_SIZE, 0, (sockaddr*)&remote_addr, &nAddrlen);
		cout << "Filename: " << fileName << endl;
		printf("Please enter the file address to receive: ");
		int readLen = 0;
		char desFileName[100];
		cin >> desFileName;
		errno_t err;
		FILE* fp;
		if ((err = fopen_s(&fp, desFileName, "wb")) < 0) {
			cout << "Create failed." << endl;
			return -1;
		}
		int length;
		while ((length = recvfrom(server, recvBuff, BUF_SIZE, 0, (sockaddr*)&remote_addr, &nAddrlen))) {
			if (!strcmp(recvBuff, "end"))//���ս�����Ϣ
				break;
			if (length == 0) {
				cout << "An error occurred while receiving." << endl;
				return -1;
			}
			int ret = fwrite(recvBuff, 1, length, fp);
			if (ret < length) {
				cout << "Write failed." << endl;
				return -1;
			}
			sendto(server, "success", sizeof("success") + 1, 0, (SOCKADDR*)&remote_addr, sizeof(remote_addr));
		}
		cout << "Successfully received!" << endl;
		fclose(fp);
	}
	closesocket(server);
	WSACleanup();
	return 0;
}
