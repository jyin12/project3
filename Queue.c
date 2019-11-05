//
// Created by Jiayi Yin on 11/3/2019.
//

#include "Server.h"

int front;
int back = 0;

//adding socket descriptors to the queue
void Enqueue(int fd, int Queue[])
{
    if(!isEmpty(Queue) & !isFull(Queue))
    {
        if(back == -1)
            shiftQ(Queue);
        back++;
        Queue[back] = fd;
    }

    else if(isEmpty(Queue))
    {
        Queue[back] = fd;
    }
    else
    {
        printf("Queue is full.\n");
        return;
    }
    queueLength++;
}

//removing socket descriptors from queue
int Dequeue(int Queue[])
{
    if(isEmpty(Queue))
        printf("Empty list.\n");
    int i;
    if(!isEmpty(Queue))
    {
        i = Queue[back];
        Queue[front] = -1;
        front++;
        queueLength--;
    }
    return i;
}

//checks if queue is empty (for the log file)
int isEmpty(int Queue[]){
    if(queueLength == 0){
        return 1;
    }
    return 0;
}

//checks if queue is full (for the log file)
int isFull(int Queue[]){
    if(queueLength == Qsize){
        return 1;
    }
    return 0;
}

int checkFront(int Queue[]){
    return Queue[front];
}

void shiftQ(int Queue[]){
    int i = 0;
    while(front <= back){
        Queue[i] = front;
        i++;
        front++;
    }
    front = 0;
}

