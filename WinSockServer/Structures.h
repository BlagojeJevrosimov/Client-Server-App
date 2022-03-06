#pragma once
#include <ws2tcpip.h>
#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_MESSAGE_LEN 100
#define MAX_TOPIC_LEN 30

typedef struct Message{
	char topic[MAX_TOPIC_LEN];
	char messageText[MAX_MESSAGE_LEN];

}MESSAGE;
typedef struct Message_Queue {
	MESSAGE* messages;
	int front;
	int rear;
	CRITICAL_SECTION criticalSection;
}MESSAGE_QUEUE;
typedef struct ListenThreadParams {
	HANDLE hSemaphore;
	MESSAGE_QUEUE* messageQueue;
}LISTENTHREADPARAMS;