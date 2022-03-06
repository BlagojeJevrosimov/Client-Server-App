#pragma once
#include "Structures.h"
#include <ws2tcpip.h>
#include <stdio.h>
#define MAX_CLIENTS 10

void init_list(SUB_LIST** head);
SUB_LIST* create_new_item(char topic[MAX_TOPIC_LEN]);
void add_to_list(SUB_LIST* novi, SUB_LIST** head);
SUB_LIST* findTopic(char topic[MAX_TOPIC_LEN], SUB_LIST** head);
