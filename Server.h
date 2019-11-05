//
// Created by Jiayi Yin on 10/31/2019.
//

#ifndef UNTITLED3_SERVER_H
#define UNTITLED3_SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>

#define DICTIONARY "dict.txt"
#define NUM_WORKERS 2
#define PORTNUM 8888
#define Qsize 2


#endif //UNTITLED3_SERVER_H

//thread stuff
pthread_mutex_t queueMutex;
pthread_cond_t QisFull;
pthread_cond_t QisEmpty;

int dictCount;
char* clientMessage;
char* spellCorrect;
char* spellIncorrect;
char* msgPrompt;
char* errorMessage;
char* closeMessage;
int Queue[Qsize]; //queue for 2 file descriptors
int queueLength;
pthread_t threads[NUM_WORKERS];

int open_listenfd(int port);
int readAndCheck(char* word);
void *simpleServer(int portNum);
void logQueue(char* message);
void* workerThread();

void Enqueue(int fc, int Queue[]);
int Dequeue(int Queue[]);
int checkFront(int Queue[]);
int isEmpty(int Queue[]);
int isFull(int Queue[]);
void shiftQ(int Queue[]);
