#pragma once
#include <ws2tcpip.h>
#include <stdio.h>
#define MAX_MESSAGE_LEN 100
#define MAX_TOPIC_LEN 30
#define IP_LEN 10

typedef struct Message {
	char topic[MAX_TOPIC_LEN];
	char messageText[MAX_MESSAGE_LEN];
}MESSAGE;
typedef struct ClientInfo{
	char ipAddress[IP_LEN];
	short listenPort;
}CLIENT_INFO;