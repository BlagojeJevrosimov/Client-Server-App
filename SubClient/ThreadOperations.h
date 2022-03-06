#pragma once
#include <ws2tcpip.h>
#include <stdio.h>
#include "Structures.h"

#define DEFAULT_BUFLEN 512
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define DEFAULT_PORT "27019"

bool InitializeWindowsSockets();
DWORD WINAPI MessageListenThread(LPVOID lpParam);