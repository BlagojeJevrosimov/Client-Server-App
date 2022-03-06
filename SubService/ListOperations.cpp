#include "ListOperations.h"
void init_list(SUB_LIST** head) {
	*head = NULL;
}
SUB_LIST* create_new_item(char topic[MAX_TOPIC_LEN]) {
	SUB_LIST* subList = (SUB_LIST*)malloc(sizeof(SUB_LIST));
	strcpy_s(subList->topic, topic);
	for (int i = 0;i < MAX_CLIENTS;i++) {
		subList->subPorts[i] = -1;
	}
	subList->numSubs = 0;
	subList->next = NULL;
	return subList;
}
void add_to_list(SUB_LIST* novi, SUB_LIST** head) {
	if (*head == NULL) { // list is empty
		*head = novi;
		return;
	}
	add_to_list(novi, &((*head)->next));
}
SUB_LIST* findTopic(char topic[MAX_TOPIC_LEN],SUB_LIST **head) {
	SUB_LIST* temp = *head;
	while (temp != NULL) {
		if (strcmp(topic, temp->topic) == 0) {
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}
