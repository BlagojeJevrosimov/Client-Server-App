#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "ThreadOperations.h"
#include "Structures.h"
#include "QueueOperations.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27017"

int  main()
{
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket;

    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[DEFAULT_BUFLEN];

    MESSAGE* tempMessage = NULL;

    acceptedSocket = NULL;
    //Semaphore initialization
    HANDLE hSemaphore;
    hSemaphore = CreateSemaphore(0, 0, QUEUESIZE, NULL);

    //Message queue
    MESSAGE_QUEUE* messageQueue = InitQueue();

    //List of topics and subscribed sockets
    //Topics added manually
    SUB_LIST* subList;
    init_list(&subList);
    char listItem[30] = "Politika";
    SUB_LIST* novi = create_new_item(listItem);
    add_to_list(novi, &(subList));
    strcpy_s(listItem, "Nauka");
    novi = create_new_item(listItem);
    add_to_list(novi, &(subList));
    strcpy_s(listItem, "Zabava");
    novi = create_new_item(listItem);
    add_to_list(novi, &(subList));

    //Subscriber listen thread parameters
    SUBLISTENTHREADPARAMS* subListenParams = (SUBLISTENTHREADPARAMS*)malloc(sizeof(SUBLISTENTHREADPARAMS));
    subListenParams->subList = subList;
    
    //SubSend thread parameters initialization
    SUB_SEND_THREAD_PARAMS* subSendParams = (SUB_SEND_THREAD_PARAMS*)malloc(sizeof(SUB_SEND_THREAD_PARAMS));
    subSendParams->messageQueue = messageQueue;
    subSendParams->subList = subList;
    subSendParams->hSemaphore = hSemaphore;

    //SubListenThread initialization
    DWORD subListenThread;
    HANDLE hSubListenThread;
    hSubListenThread = CreateThread(NULL, 0, &SubListenThread, subListenParams, 0, &subListenThread);
    //SubSend thread initialization
    DWORD subSendThread;
    HANDLE hSubSendThread;
    hSubSendThread = CreateThread(NULL, 0, &SubSendThread, subSendParams, 0, &subSendThread);

    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
        return 1;
    }

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_STREAM,  // stream socket
        IPPROTO_TCP); // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    } 

    printf("SubService initialized, waiting for PubService.\n");

    acceptedSocket = accept(listenSocket,NULL,NULL);
    


    while (true)
    {
        // recv koji prima poruku od publishera, nit je zaustavljena sve dok poruka ne pristigne zbog blokirajuceg moda soketa
        iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult <= 0) {
            printf("Error in recv func in SubService");
            closesocket(acceptedSocket);
            WSACleanup();
            return 1;
        }
        tempMessage = (MESSAGE*)recvbuf;
        Enqueue(messageQueue, tempMessage);
        //Pustanje niti za slanje subovima da uradi jednu iteraciju
        ReleaseSemaphore(subSendParams->hSemaphore, 1, NULL);
    }


    closesocket(listenSocket);

    WSACleanup();

    return 0;
}

