#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>

const char CONNECTIONMSG[] = "You connected to summation server.\n";
const char ASK1ST[] = "Please enter first number: \n";
const char ASK2ND[] = "Please enter second number: \n";

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    if (2 != argc) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(-1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (simpleSocket == -1) {
        fprintf(stderr, "Could not create a socket!\n");
        exit(-1);
    }
    else {
      fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

    int val = 1;
    int result = setsockopt(simpleSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
    if (result < 0) {
        perror("reuse");
        exit(-1);
    }

    /* set up the address structure */
    /* use INADDR_ANY to bind to all local addresses */
    bzero(&simpleServer, sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /* bind to the address and port with our socket */
    returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));
    if (returnStatus == 0) {
        fprintf(stderr, "Bind completed!\n");
    }
    else {
        fprintf(stderr, "Could not bind to address!\n");
        close(simpleSocket);
        exit(-1);
    }

    /* let's listen on the socket for connections */
    returnStatus = listen(simpleSocket, 5);
    if (returnStatus == -1) {
        fprintf(stderr, "Cannot listen on socket!\n");
        close(simpleSocket);
        exit(-1);
    }

    while (1)
    {
        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        u_int clientNameLength = sizeof(clientName);

        /* wait here */
        simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);
        if (simpleChildSocket == -1) {
            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(-1);
        }
        else{
            fprintf(stderr, "Connection established.\n");
        }

        /* since i am sending ascii char there is no need to use the host-to-network conversion tools */

        /* handle the new connection request */
        int firstNum = 0;
        int secondNum = 0;
        int resultNum = 0;
        char buffer[256] = {0};

        /* write out message to the client */
        write(simpleChildSocket, CONNECTIONMSG, strlen(CONNECTIONMSG) + 1);

        /* write out message to the client */
        write(simpleChildSocket, ASK1ST, strlen(ASK1ST) + 1);

        /* get the message from the client */
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
        if ( returnStatus > 0 ) {
            firstNum = atoi(buffer);
            fprintf(stderr,"Client send: %d\n", firstNum);
        } else {
            fprintf(stderr, "Return Status = %d \n", returnStatus);
        }

        /* write out message to the client */
        write(simpleChildSocket, ASK2ND, strlen(ASK2ND) + 1);

        /* get the message from the client */
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
        if ( returnStatus > 0 ) {
            secondNum = atoi(buffer);
            fprintf(stderr,"Client send: %d\n", secondNum);
        } else {
            fprintf(stderr, "Return Status = %d \n", returnStatus);
        }

        /* calculate result and write it to buffer */
        resultNum = firstNum + secondNum;
        sprintf(buffer,"%d + %d = %d\n",firstNum,secondNum,resultNum);

        /* write out message to the client */
        write(simpleChildSocket, buffer, strlen(buffer)+1);

        /* close the connection to the client */
        close(simpleChildSocket);
        fprintf(stderr, "Connection closed.\n");
    }

}