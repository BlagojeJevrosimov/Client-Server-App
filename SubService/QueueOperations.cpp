#include "QueueOperations.h"

MESSAGE_QUEUE* InitQueue() {

	MESSAGE* messageArray = (MESSAGE*)malloc(sizeof(QUEUESIZE));

	MESSAGE_QUEUE* queue = (MESSAGE_QUEUE*)malloc(sizeof(MESSAGE_QUEUE));
	queue->front = -1;
	queue->rear = -1;
	queue->messages = messageArray;

	InitializeCriticalSection(&(queue->criticalSection));

	return queue;
}

void Enqueue(MESSAGE_QUEUE* queue, MESSAGE* message)
{
	EnterCriticalSection(&(queue->criticalSection));

	if (queue->rear == QUEUESIZE - 1)
		printf("\nQueue is Full!!");
	else {
		if (queue->front == -1)
			queue->front = 0;
		queue->rear++;
		queue->messages[queue->rear] = *(message);
	}
	LeaveCriticalSection(&(queue->criticalSection));
}

MESSAGE Dequeue(MESSAGE_QUEUE* queue)
{
	EnterCriticalSection(&(queue->criticalSection));

	MESSAGE message;

	if (queue->front == -1) {

		strcpy_s(message.messageText, NULL);
		strcpy_s(message.topic, NULL);

	}
	else {
		printf("\nDequeue operation sucessful\n");
		strcpy_s(message.topic, (queue->messages[queue->front]).topic);
		strcpy_s(message.messageText, (queue->messages[queue->front]).messageText);


		queue->front++;
		if (queue->front > queue->rear)
			queue->front = queue->rear = -1;
	}

	LeaveCriticalSection(&(queue->criticalSection));


	return message;
}

void PrintQueue(MESSAGE_QUEUE* queue)
{
	EnterCriticalSection(&(queue->criticalSection));

	if (queue->rear == -1)
		printf("\nQueue is Empty!!!");
	else {
		int i;
		printf("\nQueue elements are:\n");
		for (i = queue->front; i <= queue->rear; i++)
			printf("%d. Topic: %s \tMessage:%s\n", i, queue->messages[i].topic, queue->messages[i].messageText);
	}
	printf("\n");

	LeaveCriticalSection(&(queue->criticalSection));
}