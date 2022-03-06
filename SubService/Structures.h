#pragma once
#include <ws2tcpip.h>
#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_MESSAGE_LEN 100
#define MAX_TOPIC_LEN 30
#define MAX_CLIENTS 10
#define IP_LEN 10

typedef struct Message {
	char topic[MAX_TOPIC_LEN];
	char messageText[MAX_MESSAGE_LEN];

}MESSAGE;
typedef struct Message_Queue {
	MESSAGE* messages;
	int front;
	int rear;
	CRITICAL_SECTION criticalSection;
}MESSAGE_QUEUE;
typedef struct Sub_List{
	char topic[MAX_TOPIC_LEN];
	short subPorts[MAX_CLIENTS];
	int numSubs;
	CRITICAL_SECTION criticalSection;
	struct Sub_List* next;
}SUB_LIST;
typedef struct SubListenThreadParams {
	SUB_LIST* subList;
}SUBLISTENTHREADPARAMS;
typedef struct SubSendThreadParams {
	SUB_LIST* subList;
	MESSAGE_QUEUE* messageQueue;
	HANDLE hSemaphore;
}SUB_SEND_THREAD_PARAMS;
typedef struct ClientInfo {
	char ipAddress[IP_LEN];
	short listenPort;
}CLIENT_INFO;