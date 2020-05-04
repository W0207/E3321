#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <winsock2.h>
#pragma warning(disable:4996)

#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET m_Client;
sockaddr_in serAddr;
void SendFile();

int main()
{
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(sockVersion, &data) != 0)
    {
        return 0;
    }
    while (true) {
        m_Client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_Client == INVALID_SOCKET)
        {
            printf("invalid socket!");
            return 0;
        }

        serAddr.sin_family = AF_INET;
        serAddr.sin_port = htons(8888);
        serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
        if (connect(m_Client, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
        {  //连接失败 
            printf("connect error !");
            closesocket(m_Client);
            return 0;
        }
        SendFile();
        closesocket(m_Client);
    }
    WSACleanup();
    return 0;

}

void SendFile() {
    int haveSend = 0;
    const int bufferSize = 1024;
    char buffer[bufferSize] = { 0 };
    int readLen = 0;
    printf("请输入要传送的文件地址：");
    string srcFileName;
    cin >> srcFileName;
    ifstream srcFile;
    srcFile.open(srcFileName.c_str(), ios::binary);
    if (!srcFile) {
        return;
    }
    while (!srcFile.eof()) {
        srcFile.read(buffer, bufferSize);
        readLen = srcFile.gcount();
        send(m_Client, buffer, readLen, 0);
        haveSend += readLen;
    }
    srcFile.close();
    cout << "send: " << haveSend << "B" << endl;
}