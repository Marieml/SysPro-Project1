/*
	Name: Maria Miliou
	A.M.: 1115201300101
*/

#ifndef QUEUE_H
#define	QUEUE_H

#include <stdbool.h>
#include"item.h"

typedef struct queue* Queue;

Queue queue_init();
bool queue_push(Queue,item_t);
item_t queue_pop(Queue);
char *queue_find(Queue,pid_t);
bool queue_isempty(Queue);
void queue_destroy(Queue);

int worker(int, char*);
#endif
