#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "Structures.h"
#include "ListOperations.h"
#include "CommonOperations.h"
#include "QueueOperations.h"

#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 10
#define DEFAULT_PORT2 "27018"

bool InitializeWindowsSockets();
DWORD WINAPI SubListenThread(LPVOID lpParam);
DWORD WINAPI SubSendThread(LPVOID lpParam);