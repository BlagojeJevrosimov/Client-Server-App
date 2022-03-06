#pragma once
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_MESSAGE_LEN 100
#define MAX_TOPIC_LEN 30
typedef struct Message {
	char topic[MAX_TOPIC_LEN];
	char messageText[MAX_MESSAGE_LEN];

}MESSAGE;