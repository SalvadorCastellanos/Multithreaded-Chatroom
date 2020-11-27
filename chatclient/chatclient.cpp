//---------------------------------------------------------------------------
// chatclient.cpp
// Salvador Castellanos
// scastel3@ucsc.edu
// 11/25/20
// Description: This is a  chatroom client. I created this to get a better 
// understanding of the topic of multithreading and semaphores that I am 
// currently learning in my Principles of Computer Systems Design course.
//---------------------------------------------------------------------------

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>  
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#define MAX 800 
#define PORT 8080 
#define SA struct sockaddr 

void process(int sockfd)
{
    char buff[MAX];
    int n;
    int flag = 0;
    for (;;) {
        //flag = 0 for first message (name)
        if (flag == 0){
            flag = 1;
            bzero(buff, sizeof(buff));
            n = 0;

            //get the name of the user
            fprintf(stdout,"Enter your name:");
            while (1){
                if((buff[n++] = getchar()) == '\n'){
                    break;
                }
            }

            //send name to the server
            send(sockfd, buff, sizeof(buff),0);
            if ((strncmp(buff, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
            bzero(buff, sizeof(buff));

            //receive Enter a Message: from server
            recv(sockfd, buff, sizeof(buff),0);
            printf("%s", buff);
        }else{
            bzero(buff, sizeof(buff));
            n = 0;

            //read input into buff and send message to server
            while ((buff[n++] = getchar()) != '\n');
            send(sockfd, buff, sizeof(buff),0);
            if ((strncmp(buff, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
            bzero(buff, sizeof(buff));

            //receive Enter a Message: from server
            recv(sockfd, buff, sizeof(buff),0);
            printf("%s", buff);
        }
        
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        exit(0);
    }
    else
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    //Connect to server, print success or failure
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("Connection with the server failed.\n"); 
        exit(0); 
    } 
    else{
        printf("Connected to the server!\n");
    }

    // function for chat
    process(sockfd);

    // close the socket when done
    close(sockfd);
}


