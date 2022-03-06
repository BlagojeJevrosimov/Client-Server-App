#include "CommonOperations.h"

bool Subscribe(CLIENT_INFO ci, SUB_LIST* subList) {
	if (subList->numSubs == MAX_CLIENTS)
		return false;

	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
	for (int i = 0; i < subList->numSubs;i++) {
		if (ci.listenPort == subList->subPorts[i]) {
			LeaveCriticalSection(&cs);
			return false;
		}
	}
	subList->subPorts[subList->numSubs] = ci.listenPort;
	subList->numSubs++;
	LeaveCriticalSection(&cs);
	DeleteCriticalSection(&cs);
	return true;

}
void Unsubscribe(CLIENT_INFO ci, SUB_LIST** subList) {
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);

	SUB_LIST* temp = *subList;
	int initNubSub;
	while (temp != NULL) {
		initNubSub = temp->numSubs;
		for (int i = 0; i < initNubSub; i++) {
			if (ci.listenPort == temp->subPorts[i]) {

				temp->subPorts[i] = -1;
				temp->numSubs--;

			}
		}
		temp = temp->next;
	}
	LeaveCriticalSection(&cs);
	DeleteCriticalSection(&cs);
}