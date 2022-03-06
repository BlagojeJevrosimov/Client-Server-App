#include "ThreadOperations.h";
#include "QueueOperations.h"


DWORD WINAPI PubListenThread(LPVOID lpParam) {
    //nit koja prima pub-ove i stavlja njihove objave na red
    LISTENTHREADPARAMS* params = (LISTENTHREADPARAMS*)lpParam;

    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        acceptedSocket[i] = INVALID_SOCKET;
    }

    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[DEFAULT_BUFLEN];
    int numClients = 0;

    MESSAGE* tempMessage = NULL;

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

    unsigned long mode = 1;
    iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
    printf("PubService initialized, waiting for clients.\n");

    fd_set readfds;
    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    while (true)
    {
        FD_ZERO(&readfds);
        if (numClients < MAX_CLIENTS) {
            FD_SET(listenSocket, &readfds);
        }
        for (int i = 0; i < numClients; i++) {
            FD_SET(acceptedSocket[i], &readfds);
        }

        iResult = select(0, &readfds, NULL, NULL, &timeVal);
        if (iResult == 0) {}
        else if (iResult == SOCKET_ERROR) {
        }
        else {
            if (FD_ISSET(listenSocket, &readfds)) {
                struct sockaddr_in sa = { 0 };
                socklen_t socklen = sizeof sa;
                acceptedSocket[numClients] = accept(listenSocket, (struct sockaddr*)&sa, &socklen);
                if (acceptedSocket[numClients] == INVALID_SOCKET) {
                    printf("Accept failed\n");
                }
                else {
                    printf("Primio klijent %d\n", numClients);
                    numClients++;
                }
            }

            for (int i = 0; i < numClients; i++) {
                if (FD_ISSET(acceptedSocket[i], &readfds)) {
                    iResult = recv(acceptedSocket[i], recvbuf, DEFAULT_BUFLEN, 0);
                    if (iResult > 0)
                    {
                        // Primanje objave od klijenta i stavljanje na red
                        tempMessage = (MESSAGE*)recvbuf;
                        printf("Primio od klijenta %d : %s\t: %s\n", i, tempMessage->topic, tempMessage->messageText);
                        Enqueue(params->messageQueue, tempMessage);
                        //pustanje niti za slanje objava Sub servisu u rad
                        ReleaseSemaphore(params->hSemaphore, 1, NULL);
                    }
                    else if (iResult == 0)
                    {
                        // connection was closed gracefully
                        printf("Connection with client closed.\n");
                        closesocket(acceptedSocket[i]);
                        if (i < numClients) {
                            for (int j = i; j < numClients; j++) {
                                acceptedSocket[j] = acceptedSocket[j + 1];
                            }
                        }
                        numClients--;
                    }
                    else
                    {
                        printf("Connection with client closed.\n");
                        closesocket(acceptedSocket[i]);
                        if (i < numClients) {
                            for (int j = i; j < numClients; j++) {
                                acceptedSocket[j] = acceptedSocket[j + 1];
                            }
                        }
                        numClients--;
                    }
                }
            }
        }

    }
        closesocket(listenSocket);

        WSACleanup();

        return 0;
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

