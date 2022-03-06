#include "ThreadOperations.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

DWORD WINAPI SubListenThread(LPVOID lpParam) {
    //thread koji prima nove subove i slusa za zahteve za pretplatu
    SUBLISTENTHREADPARAMS* params = (SUBLISTENTHREADPARAMS*)lpParam;

    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        acceptedSocket[i] = INVALID_SOCKET;
    }
    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[DEFAULT_BUFLEN];
    //broj klijenata na serveru trenutno
    int numClients = 0;
    
    SUB_LIST* temp = NULL;

    CLIENT_INFO clientInfo[MAX_CLIENTS];
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
    iResult = getaddrinfo(NULL, DEFAULT_PORT2, &hints, &resultingAddress);
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
    printf("SubListenThread initialized, waiting for client sub requests.\n");

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

                acceptedSocket[numClients] = accept(listenSocket, NULL, NULL);
                if (acceptedSocket[numClients] == INVALID_SOCKET) {

                    printf("Accept failed\n");
                }
                else {
                    printf("Client %d succesfuly connected to SubService.\n", numClients);
                    
                    unsigned long mode = 0;
                    iResult = ioctlsocket(acceptedSocket[numClients], FIONBIO, &mode);
                    
                    iResult = recv(acceptedSocket[numClients], recvbuf, DEFAULT_BUFLEN, 0);
                    if (iResult < 0) {
                        printf("Client info not sent.\n");
                        closesocket(acceptedSocket[numClients]);
                        continue;
                    }
                    //primam informacije o ip adresi i portu na kome klijent prima poruke
                    CLIENT_INFO* ci = (CLIENT_INFO*)recvbuf;
                    strcpy_s(clientInfo[numClients].ipAddress, ci->ipAddress);
                    clientInfo[numClients].listenPort = ci->listenPort;

                    numClients++;
                    
                }
            }
            for (int i = 0; i < numClients; i++) {
                if (FD_ISSET(acceptedSocket[i], &readfds)) {
                    iResult = recv(acceptedSocket[i], recvbuf, DEFAULT_BUFLEN, 0);
                    if (iResult > 0)
                    {
                        //Ako ne postoji taj topic, nije dozvoljena pretplata
                        EnterCriticalSection(&cs);
                        temp = findTopic(recvbuf, &(params->subList));
                        LeaveCriticalSection(&cs);

                        if (temp == NULL) {
                            iResult = send(acceptedSocket[i], "1", sizeof("1"), 0);
                            if (iResult == SOCKET_ERROR)
                            {
                                printf("send failed with error: %d\n", WSAGetLastError());
                                closesocket(acceptedSocket[i]);
                                WSACleanup();
                                return 1;
                            }
                            printf("Client %d: request denied, no such topic.\n",i);
                        }
                        else {

                            //pretplata na trazenu temu
                            if (Subscribe(clientInfo[i], temp)) {
                                iResult = send(acceptedSocket[i], "0", sizeof("0"), 0);
                                if (iResult == SOCKET_ERROR) {
                                    printf("send failed with error: %d\n", WSAGetLastError());
                                    closesocket(acceptedSocket[i]);
                                    WSACleanup();
                                    return 1;
                                }
                                printf("Client %d: subscribed to %s topic succesfuly.\n", i, recvbuf);
                            }
                            else {
                                //vec je pretplacen na tu temu
                                iResult = send(acceptedSocket[i], "2", sizeof("2"), 0);
                                if (iResult == SOCKET_ERROR)
                                {
                                    printf("send failed with error: %d\n", WSAGetLastError());
                                    closesocket(acceptedSocket[i]);
                                    WSACleanup();
                                    return 1;
                                }
                                printf("Client %d: already subscribed to %s topic.\n", i, recvbuf);
                            }
                        }

                    }
                    else if (iResult == 0)
                    {
                        // connection was closed gracefully
                        //ocisti sve subscrajbove njegove
                        printf("Connection with client closed.\n");
                        //azuriram lisu koja cuva podatke o tome ko je pretplacen na koju temu
                        Unsubscribe(clientInfo[i], &params->subList);
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
                        //ocisti sve subscrajbove njegove
                        printf("Connection with client closed.\n");
                        //azuriram lisu koja cuva podatke o tome ko je pretplacen na koju temu
                        Unsubscribe(clientInfo[i], &params->subList);
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
DWORD WINAPI SubSendThread(LPVOID lpParam) {
        //nit koja salje poruku svim sub-ovima na prosledjenu temu
        SUB_SEND_THREAD_PARAMS* params = (SUB_SEND_THREAD_PARAMS*)lpParam;
        while (true) {
            //cnit ceka dozvolu kako ne bi radila u prazno
            WaitForSingleObject(params->hSemaphore, INFINITE);
            MESSAGE message = Dequeue(params->messageQueue);
            // socket used to communicate with server
            SOCKET connectSocket = INVALID_SOCKET;
            // variable used to store function return value
            int iResult;

            char recvbuf[DEFAULT_BUFLEN];

            if (InitializeWindowsSockets() == false)
            {
                // we won't log anything since it will be logged
                // by InitializeWindowsSockets() function
                return 1;
            }

            //provera da li dati topic postoji
            SUB_LIST* subList = findTopic(message.topic, &(params->subList));
            if (subList == NULL) {
                printf("Unexpected error in sendThread.\n");
                return 0;
            }

            for (int i = 0; i < subList->numSubs; i++) {
                //konekcija sa klijentom, zatim slanje, pa gasenje konekcije i tako u krug dok sve pretplacene klijente ne opsluzimo
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
                serverAddress.sin_port = htons(subList->subPorts[i]);
                // connect to server specified in serverAddress and socket connectSocket
                if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
                {
                    printf("Unable to connect to server.\n");
                    closesocket(connectSocket);
                    WSACleanup();
                }
                
                iResult = send(connectSocket, (char*)&message, sizeof(MESSAGE), 0);

                if (iResult == SOCKET_ERROR)
                {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(connectSocket);
                    WSACleanup();
                    return 1;
                }
                closesocket(connectSocket);
            }
            
        }
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

