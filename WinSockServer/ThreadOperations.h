#pragma once
#include "Structures.h"
#include <ws2tcpip.h>
#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 10
#define DEFAULT_PORT "27016"

bool InitializeWindowsSockets();
DWORD WINAPI PubListenThread(LPVOID lpParam);