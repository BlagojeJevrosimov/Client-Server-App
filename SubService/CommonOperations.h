#pragma once
#include "Structures.h"
#include <ws2tcpip.h>
#include <stdio.h>
#define MAX_TOPIC_LEN 30

bool Subscribe(CLIENT_INFO ci, SUB_LIST* subList);
void Unsubscribe(CLIENT_INFO ci, SUB_LIST** subList);