#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "Structures.h"
#include "ListOperations.h"
#include "ThreadOperations.h"
#include "QueueOperations.h"

#define DEFAULT_BUFLEN 512

bool InitializeWindowsSockets();

int  main(void) {
    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // message to send
    char messageToSend[50];
    char recvbuf[DEFAULT_BUFLEN];
    //Semafori
    HANDLE hSemaphore;
    hSemaphore = CreateSemaphore(0, 0, QUEUESIZE, NULL);
    //Queue
    MESSAGE_QUEUE* messageQueue = InitQueue();
    //Threadovi
    DWORD listenThread;
    HANDLE hListenThread;
    LISTENTHREADPARAMS* listenThreadParams = (LISTENTHREADPARAMS*)malloc(sizeof(LISTENTHREADPARAMS));
    listenThreadParams->hSemaphore = hSemaphore;
    listenThreadParams->messageQueue = messageQueue;
    hListenThread = CreateThread(NULL, 0, &PubListenThread, listenThreadParams, 0, &listenThread);
    

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
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(27017);
    // connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to SubService.\n");
        closesocket(connectSocket);
        WSACleanup();
        return -1;
    }
    while (true) {
        // nit ceka dok se ne stavi objava na red nakon cega je salje Sub servisu
        WaitForSingleObject(hSemaphore, INFINITE);
        MESSAGE m = Dequeue(messageQueue);
        iResult = send(connectSocket, (char*)&m,sizeof(MESSAGE), 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }
    }
    // cleanup
    closesocket(connectSocket);
    WSACleanup();
    return 0;
}
