/*
	Name Maria Miliou
	A.M.:1115201300101
*/


#include<stdlib.h>
#include<stdbool.h>
#include"item.h"
#include"queue.h"

/*-----Implementation of queue using linked list structure-----*/

struct node
{
	item_t item;
	struct node *next;
};


struct queue{
	struct node *list_head;
	struct node *list_tail;
};

//Initialization
Queue queue_init(){
	Queue q = malloc(sizeof(Queue));
	q->list_head = NULL;
	q->list_tail = NULL;
	return q;
}

//Push item returns true if succeed otherwise false
bool queue_push(Queue q,item_t it)
{
	struct node *newNode = malloc(sizeof(struct node));

	if(newNode==NULL)
		return false;

	newNode->item=it;
	newNode->next = NULL;

	if(q->list_tail!=NULL)
		q->list_tail->next=newNode;

	q->list_tail=newNode;

	if(q->list_head==NULL) //First insert
		q->list_head=q->list_tail;
	return true;
}

//Pop item and return
item_t queue_pop(Queue q)
{
	struct node *temp;
	item_t item;

	item = q->list_head->item;
	temp = q->list_head->next;

	free(q->list_head);
	q->list_head=temp;

	return item;
}

//Returns true id queue is empty, otherwise false
bool queue_isempty(Queue q)
{
	return (q->list_head==NULL);
}

//Destroy queue
void queue_destroy(Queue q){
	while(!queue_isempty(q)){
		queue_pop(q);
	}
	free(q);
}

//Customized find
//Search fifo name by pid
char *queue_find(Queue q, pid_t pid){

	struct node *temp = q->list_head;
	while(temp!=NULL){
		if(temp->item.pid == pid)
			return temp->item.fifo;
		temp=temp->next;
	}

}
