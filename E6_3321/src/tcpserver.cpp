#include <stdio.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <winsock2.h>
#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
using namespace std;

SOCKET m_Client;

void RecvFile();

sockaddr_in remoteAddr;

int main()
{
    //初始化WSA  
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }

    //创建套接字  
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (slisten == INVALID_SOCKET)
    {
        printf("socket error !");
        return 0;
    }

    //绑定IP和端口  
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8888);
    sin.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        printf("bind error !");
    }

    //开始监听  
    if (listen(slisten, 5) == SOCKET_ERROR)
    {
        printf("listen error !");
        return 0;
    }

    //循环接收数据  
    int nAddrlen = sizeof(remoteAddr);
    char revData[255];
    while (true)
    {
        printf("等待连接...\n");
        m_Client = accept(slisten, (SOCKADDR*)&remoteAddr, &nAddrlen);
        if (m_Client == INVALID_SOCKET)
        {
            printf("accept error !");
            continue;
        }
        printf("接受到一个连接：IP:%s \r\n", inet_ntoa(remoteAddr.sin_addr));
        RecvFile();
        closesocket(m_Client);
    }

    closesocket(slisten);
    WSACleanup();
    return 0;
}
void RecvFile() {
    const int bufferSize = 1024;
    char buffer[bufferSize] = { 0 };
    printf("请输入要接收的文件地址：");
    int readLen = 0;
    string desFileName;
    cin >> desFileName;
    ofstream desFile;
    desFile.open(desFileName.c_str(), ios::binary);
    if (m_Client == INVALID_SOCKET)
    {
        printf("失去连接：IP:%s 请重新传输\r\n", inet_ntoa(remoteAddr.sin_addr));
        return;
    }
    if (!desFile)
    {
        return;
    }
    do
    {
        readLen = recv(m_Client, buffer, bufferSize, 0);
        if (readLen == 0)
        {
            break;
        }
        else
        {
            desFile.write(buffer, readLen);
        }
    } while (true);
    desFile.close();
}
