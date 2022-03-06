#include "ThreadOperations.h"


DWORD WINAPI MessageListenThread(LPVOID lpParam) {

    CLIENT_INFO* ci = (CLIENT_INFO*)lpParam;
    strcpy_s(ci->ipAddress, "127.0.0.1");
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
    //Pronalazimo slobodan port za pokretanje listen socketa na njemu
    char port[6];
    strcpy_s(port, DEFAULT_PORT);
    int i = atoi(port);

    getaddrinfo(NULL, port, &hints, &resultingAddress);
    while (bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen) < 0) {
        i++;
        _itoa_s(i, port, 10);
        getaddrinfo(NULL, port, &hints, &resultingAddress);
    }
    ci->listenPort = i;
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

    printf("Listen thread initialized, waiting for messages on subscribed topics.\n");
    while (true) {
        //nit prima konekciju na listen soceket, zatim poruku od Sub servisa, nakon ceka nastavlja da slusa
    acceptedSocket = accept(listenSocket, NULL, NULL);

        iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult <= 0) {
            printf("Error in recv func in SubService");
            closesocket(acceptedSocket);
            WSACleanup();
            return 1;
        }
        tempMessage = (MESSAGE*)recvbuf;
        printf("\nMessage on topic %s: %s\n", tempMessage->topic, tempMessage->messageText);

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