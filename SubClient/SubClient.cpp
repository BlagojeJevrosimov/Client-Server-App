#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "ThreadOperations.h"

#define WIN32_LEAN_AND_MEAN
#define MAX_TOPIC_LEN 30
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27018

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.

int __cdecl main()
{
    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // message to send
    char recvbuf[DEFAULT_BUFLEN];

    char topic[MAX_TOPIC_LEN];
    CLIENT_INFO* ci = (CLIENT_INFO*)malloc(sizeof(CLIENT_INFO));
    //Thread initialization
    DWORD messageListenThread;
    HANDLE hMessageListenThread;
    hMessageListenThread = CreateThread(NULL, 0, &MessageListenThread, ci, 0, &messageListenThread);

    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
        return 1;
    }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    //InetPtonW(AF_INET, _T("127.0.0.1"), &serverAddress.sin_addr.s_addr);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(DEFAULT_PORT);
    // connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    iResult = send(connectSocket, (const char*)ci, sizeof(CLIENT_INFO), 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    while (true) {
        //Slanje topica na koji zelimo da se pretplatimo
        printf("Subscribe to topic: ");
        scanf_s("%s", topic,MAX_TOPIC_LEN);
        iResult = send(connectSocket, topic, sizeof(topic), 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }
        iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult <= 0) {
        
            closesocket(connectSocket);
            printf("Error in recv function");
            WSACleanup();
            return 1;
        }
        if (recvbuf[0] == '0') {
            printf("Succesfuly subscribed to topic %s.\n",topic);
        }
        if (recvbuf[0] == '1') {
            printf("No topic %s on server.\n", topic);
        }
        if (recvbuf[0] == '2') {
            printf("Already subscribed to topic %s.\n", topic);
        }
    }
    // cleanup
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}

