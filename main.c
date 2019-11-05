#include "Server.h"
int dictCount;
char c[100000];
char* userDict;
int main(int argc , char *argv[])
{
    queueLength = 0;
    int portNum;

    //nothing specified use default port number, default dict
    if(argc == 1){
        portNum = PORTNUM; //portnum is 8888
        userDict = DICTIONARY;
    }

    //dictionary specified, user default port
    else if(argc == 2){
        portNum = atoi(argv[1]);
        userDict = DICTIONARY;
    }

    else if(argc > 3){
        printf("Too many arguments, exiting\n");
        exit(0);
    }

    for(int i = 0; i < NUM_WORKERS; i++){
        Queue[i] = -1;
    }

    clientMessage = "Spellchecker: type a word >>> \n";
    spellCorrect = "Correct\n";
    spellIncorrect = "Incorrect\n";
    errorMessage = "Didn't get message\n";
    closeMessage = "Goodbye!\n";

    pthread_mutex_init(&queueMutex, NULL);
    pthread_cond_init(&QisFull, NULL);
    pthread_cond_init(&QisEmpty, NULL);

    //creates the 2 threads: server and worker
    for(int i = 0; i < NUM_WORKERS; i++){
        pthread_create(&threads[i], NULL, workerThread, NULL); //NULL on 4th arg because we don't want to pass data to new thread
    }

    //socket connection
    simpleServer(portNum);
}

int readAndCheck(char* word){
    int i;
    FILE *fp;
    if((fp =  fopen(userDict, "r")) == NULL){
        printf("Error opening file");
        exit(1);
    }

    while((i = fgetc(fp)) != EOF) {
        fscanf(fp, "%s", c);
        if(strcmp(c, word) == 0)
            return 1;
        else
            return 0;
    }
    fclose(fp);
}

//from TA lab socket file
void *simpleServer(int portNum){
    //sockaddr_in holds information about the user connection.
    //We don't need it, but it needs to be pass into accept()
    struct sockaddr_in client;
    int clientLen = sizeof(client);
    int connectionSocket;
    int clientSocket;
    int bytesReturned;

    //We can't use ports below 1024 and ports above 65535 don't exist
    if(portNum < 1024 || portNum > 65535)
    {
        printf("port number is either too low (below 1024), or too high (above 65535).\n");
        exit(0);
    }

    //Does all the hard work for us
    connectionSocket = open_listenfd(portNum);
    while(1){
        if(connectionSocket == -1)
        {
            printf("Could not connect to %d, maybe try another port number?\n", portNum);
            exit(0);
        }
        //accept() waits until user connects to the server, writing information about that server
        //into sockaddr_in client.
        //If the connection is successful, we obtain a SECOND socket descriptor.
        //There are two socket descriptors being used now:
        //One by the server to listen for incoming instructions
        //The second that was just created that will be used to communicate with
        //the connected user
        if((clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) == -1)
        {
            printf("Error connecting to client\n");
            exit(0);
        }

        pthread_mutex_lock(&queueMutex); //locks thread so no other threads come in until thread is finished with its process
        printf("Connection success!\n");
        write(clientSocket, clientMessage, strlen(clientMessage)); //Now user can write their words to be spell checked

        //send()...sends a message.
        //We specify the socket we want to send, the message and it's length, the
        //last parameter are flags
        //Begin sending and receiving messages
        while(isFull(Queue)){
            pid_t id= syscall(SYS_gettid);
            printf("Queue is full; Thread %d is sleeping\n", id);
            pthread_cond_wait(&QisFull, &queueMutex); //goes to sleep until receives signal since queue is full; Consumer comes in place where they remove a socket
        }

        //adds the socket into the queue for it to be not empty (producer)
        Enqueue(clientSocket, Queue);
        printf("Just added file descriptor %d\n", clientSocket);
        pthread_cond_signal(&QisEmpty);
        pthread_mutex_unlock(&queueMutex);
    }
}


void* workerThread() {
    int bytesReturned;
    c[0] = '\0';

    while(1){
        pthread_mutex_lock(&queueMutex); //locks so no other thread come into process, until finished
        while(isEmpty(Queue)){
            pid_t id= syscall(SYS_gettid);
            printf("Thread %d connecting....\n", id);
            pthread_cond_wait(&QisEmpty, &queueMutex);
        }

        //removes socket from queue (consumer)
        int fileDesrip = Dequeue(Queue);
        printf("Just removed file descriptor %d new size is %d\n", fileDesrip, queueLength);
        pthread_cond_signal(&QisFull);
        pthread_mutex_unlock(&queueMutex);

        while(bytesReturned = read(fileDesrip, c, sizeof(c)) > 0) {
            c[strlen(c) - 1] = '\0';

            //send calls syscall write to the client from worker
            //recv call syscall read from client to the worker
            //bytesReturned = read(fileDesrip, buffer, BUF_LEN);
            //Check if got message, send message back or quit if the
            //user specified it

            //'27' is the escape key
            if(c[0] == 27){
                logQueue(closeMessage);
                close(fileDesrip);
                break;
            }

            else{
                logQueue(c);
                int checkError = readAndCheck(c);
                if(checkError == 1){
                    write(fileDesrip, spellCorrect, strlen(spellCorrect));
                    logQueue(spellCorrect);
                }
                else if(checkError == 0){
                    write(fileDesrip, spellIncorrect, strlen(spellIncorrect));
                    logQueue(spellIncorrect);
                }
                memset(c, 0, sizeof(c));

            }
        }
        if(bytesReturned == -1){
            logQueue(errorMessage);
            write(fileDesrip, errorMessage, strlen(errorMessage));
        }
    }
}

//keeps track of spell checker
void logQueue(char *message){
    FILE *fp;
    fp = fopen("logQueue.txt", "a");
    pid_t id= syscall(SYS_gettid); //Just the id of thread (keep track)

    if(fp){
        if(strstr(message, spellCorrect)){
            fprintf(fp, "%s", message);
            fprintf(fp, "%s", "\n");
        }

        else {
            fprintf(fp, "Thread %d: \n", id);
            fprintf(fp, "%s: ", message);
        }
    }
    fclose(fp);
}

