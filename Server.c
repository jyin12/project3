//
// Created by Jiayi Yin on 11/3/2019.
//
//#include "Server.h"
#include "open_listenfd.c"
//An extremely simple server that connects to a given port.
//Once the server is connected to the port, it will listen on that port
//for a user connection
//A user will connect through the telnet, the user also needs to know the port number.
//If the user connects successfully, a socket descriptor will be created/
//The socket descriptor works exactly like file descriptor, allowing us to read/write on it.
//The server will then use the socket descriptor to communicate with the user, sending and
//receiving messages

/*
 *
 * TESTER
 *
 */
int main(int argc, char** argv)
{
    //sockaddr_in holds information about the user connection.
    //We don't need it, but it needs to be pass into accept()
    struct sockaddr_in client;
    int clientLen = sizeof(client);
    int portNum = atoi(argv[1]);
    int connectionSocket;
    int clientSocket;
    int bytesReturned;
    char recvBuffer[BUF_LEN];
    recvBuffer[0] = '\0';

    portNum = atoi(argv[1]);

    //We can't use ports below 1024 and ports above 65535 don't exist
    if(portNum < 1024 || portNum > 65535)
    {
        printf("port number is either too low (below 1024), or too high (above 65535).\n");
        return -1;
    }

    //Does all the hard work for us
    connectionSocket = open_listenfd(portNum);
    if(connectionSocket == -1)
    {
        printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
        return -1;
    }

    //accept() waits until user connects to the server, writing information about that sercer
    //into sockaddr_in client.
    //If the connection is successful, we obtain a SECOND socket descriptor.
    //There are two socket descriptors being used now:
    //One by the server to listen for incoming instructions
    //The second that was just created that will be used to communicate with
    //the connected user
    if((clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) == -1)
    {
        printf("Error connecting to client\n");
        return -1;
    }

    printf("Connection success!\n");
    char* clientMessage = "Hello! I hope you can see this.\n";
    char* msgRequest = "Send me some text and I'll respond with something interesting!\nSend the escape key to close the connection.\n";
    char* msgResponse = "I actually don't have anything interesting to say...but I know you sent ";
    char* msgPrompt = ">>>";
    char* msgError = "I didn't get your message. ):\n";
    char* msgClose = "Goodbye!\n";

    //send()...sends a message.
    //we specify the socket we want to send, the message and it's length, the
    //last parameter are flags.
    send(clientSocket, clientMessage, strlen(clientMessage), 0);
    send(clientSocket, msgRequest, strlen(msgRequest), 0);

    //Begin sending and receiving messages
    while(1)
    {
        send(clientSocket, msgPrompt, strlen(msgPrompt), 0);
        //recv() will store the message from the user in the buffer, returning
        //how many bytes we receieved.
        bytesReturned = recv(clientSocket, recvBuffer, BUF_LEN, 0);

        //Check is we got a message, send a message back or quit if the
        //user specified it.
        if(bytesReturned == -1)
        {
            send(clientSocket, msgError, strlen(msgError), 0);
        }

        //'27' is the escape key.
        else if(recvBuffer[0] == 27)
        {
            send(clientSocket, msgClose, strlen(msgClose), 0);
            close(clientSocket);
            break;
        }

        else //This is where we do work to do things with the user
        {
            send(clientSocket, msgResponse, strlen(msgResponse), 0);
            send(clientSocket, recvBuffer, bytesReturned, 0);
        }
    }
    return 0;
}

