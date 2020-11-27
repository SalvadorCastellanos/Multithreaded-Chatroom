//---------------------------------------------------------------------------
// chatserver.cpp
// Salvador Castellanos
// scastel3@ucsc.edu
// 11/25/20
// Description: This is a multithreaded chatroom server. I created this
// to get a better understanding of the topic of multithreading and semaphores
// that I am currently learning in my Principles of Computer Systems Design 
// course.
//---------------------------------------------------------------------------

#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX 800 
#define PORT 8080 
#define SA struct sockaddr 
#define THREAD_POOLSIZE 16

//Function Declarations
void* start(void* arg);
void process(int sockfd);
struct Threads* getfreethread(struct Threads thread[], int size);

//Global Mutex
sem_t mainMutex;

//Threads struct
typedef struct Threads{
  int cl;
  sem_t mutex;
}Threads;


// Function designed for chat between client and server. 
void process(int sockfd) 
{ 
    int read = 0;
    char buff[MAX]; 
    char name[MAX];
    char prompt[] = "Enter a message: ";
    int flag = 0;
    // infinite loop for chat 
    while (sockfd) { 
        bzero(buff, MAX);
        fflush(stdout);
        if (flag == 0){
            recv(sockfd, name, sizeof(name),0);
            fprintf(stdout,"Welcome to the server ");
            for(int i = 0; i < (int)strlen(name)-1;i++){
                fprintf(stdout,"%c",name[i]);
            }
            fprintf(stdout,"!\n");
            fflush(stdout); 
            send(sockfd, prompt, sizeof(prompt),0);
        }else{

            // read the message from client and copy it in buffer 
            read = recv(sockfd, buff, sizeof(buff),0); 
            if (read == 0){
                for(int i = 0; i < (int)strlen(name)-1;i++){
                    fprintf(stdout,"%c",name[i]);
                }
                fprintf(stdout," has left the server.\n");
                close(sockfd); 
                fflush(stdout);
                break; 
            }
            // if msg contains "Exit" then server exit and chat ended. 
            if (strncmp("exit", buff, 4) == 0) { 
                for(int i = 0; i < (int)strlen(name)-1;i++){
                    fprintf(stdout,"%c",name[i]);
                }
                fprintf(stdout," has left the server.\n");
                close(sockfd); 
                fflush(stdout);
                break; 
            }else if (strlen(buff) == 1){
                // and send that buffer to client 
                send(sockfd, prompt, sizeof(prompt),0); 
                //fprintf(stdout," has left the server.\n"); 
                //fflush(stdout);
            }else{
                for(int i = 0; i < (int)strlen(name)-1;i++){
                    fprintf(stdout,"%c",name[i]);
                }
                fprintf(stdout,":");
                fflush(stdout); 

                // print buffer which contains the client contents 
                printf("%s", buff); 
                bzero(buff, MAX); 

                // and send that buffer to client 
                send(sockfd, prompt, sizeof(prompt),0); 
            }
        }
        flag = 1;
    } 
} 
  
// Driver function 
int main() { 
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
    
    //Create a thread pool
    pthread_t dontcare;
    struct Threads thread[THREAD_POOLSIZE];

    // initialize and start the threads
    for (int i = 0; i < THREAD_POOLSIZE; i++) {
        struct Threads* threads = thread+i;
        sem_init(&(threads->mutex), 0, 0);
        threads->cl = 0;
        pthread_create(&dontcare,0,start,threads);
    }

    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else{
        printf("Socket successfully created..\n"); 
    }

    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Listen and Verify 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else{
        printf("Server listening..\n"); 
    }
    
    // Function for chatting between client and server 
    while (1) {
        // Wait for a new connection
        connfd = accept(sockfd, NULL, NULL); 
        if (connfd < 0) { 
            printf("server acccept failed...\n"); 
            exit(0); 
        } 

        //Create a worker thread.
        struct Threads* wthread = 0;
        
        //Find a free thread to work on.
        while((wthread = getfreethread(thread,THREAD_POOLSIZE)) == 0){
            sem_wait(&mainMutex);
        }
        wthread->cl = connfd;
        //Signal thread is free again.
        sem_post(&wthread->mutex);
    }
  
    // After chatting close the socket 
    
}

void* start(void* arg){
    struct Threads *thread = (struct Threads *) arg;
    while(1){
        //Wait for a new connection
        while(thread->cl == 0){
            sem_wait(&thread->mutex);
        }
        //Process Connection
        process(thread->cl);
        //Reset thread and signal global mutex
        thread->cl = 0;
        sem_post(&mainMutex);
    }
}

//Function that returns a free thread.
Threads* getfreethread(Threads thread[], int size){
    for (int i = 0; i < size; i++){
        if (thread[i].cl == 0){
            return thread+i;
        }
    }
    return 0;
}