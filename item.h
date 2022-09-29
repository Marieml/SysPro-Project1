/*
	Name: Maria Miliou
	A.M.: 1115201300101
*/

#ifndef ITEM_H
#define ITEM_H

//Item for queue
struct Worker{
	pid_t pid;
	char fifo[15];  //fifo name
};

typedef struct Worker item_t;

#endif
