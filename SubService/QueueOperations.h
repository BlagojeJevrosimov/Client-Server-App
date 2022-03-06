#pragma once
#include "Structures.h"
#include <ws2tcpip.h>
#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define QUEUESIZE 10

MESSAGE_QUEUE* InitQueue();
void Enqueue(MESSAGE_QUEUE* queue, MESSAGE* request);
MESSAGE Dequeue(MESSAGE_QUEUE* queue);
void PrintQueue(MESSAGE_QUEUE*);