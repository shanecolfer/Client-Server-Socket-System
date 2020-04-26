#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <pthread.h>
#include <sys/fsuid.h>

pthread_mutex_t thread_lock;

void *newConnection(void *connectionsocket)
{

    //int uid;
    pthread_mutex_init(&thread_lock, NULL);

    //printf("User ID of program before: ");
    //uid = getuid();
    //printf("%d", uid);

    int received_uid;
    int received_gid;
    int uid;
    int gid;

    char message [500];
    int cs = *(int*)connectionsocket;
    int READSIZE;
    FILE *filePointer; //File pointer

    //Read UID of user
    READSIZE = read(cs, &received_uid, sizeof(received_uid));
    received_uid = ntohl(received_uid);
    uid_t uidt = received_uid;
    printf("UID received: %d\n", received_uid);

    write(cs, "UID Received\n", strlen("UID Received\n"));

    //Read GID of user
    READSIZE = read(cs, received_gid, sizeof(received_gid));
    received_gid = ntohl(received_gid);
    gid_t gidt = received_gid;
    printf("GID received: %d\n", received_gid);

    write(cs, "GID Received\n", strlen("GID Received\n"));

    //Set the file write uid of thread to match user, same with group id
    //I have tried all of the following methods to set the ID of this program with no luck,
    //It seems to change the uid of the program but I still get filesystem privilege errors!
    //setfsgid(received_gid);
    //setfsuid(received_uid);

    //setgid(gidt);
    //setuid(uidt);

    //setegid(gidt);
    //seteuid(uidt);

    printf("%d\n", getuid());
    printf("%d\n", getgid());

    //Read requested file location
    memset(message, 0, 500);
    READSIZE = recv(cs, message, 2000, 0); //Read message

    //Check for client disconnect
    if(READSIZE == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
        return 0;
    }//Check for error
    else if (READSIZE == -1)
    {
        perror("Read error");
    }

    char fileName[20]; //Declare filename variables
    strcpy(fileName, message); //Assign filename the contents of messge
    printf("File location request: %s\n", fileName); //Printing
    //printf("User ID of program after: ");
    //uid = getuid();
    //printf("%d", uid);
    write(cs, "File location request received\n", strlen("File location request received\n"));
    
    //Create file pointer with location
    filePointer = fopen(fileName, "w");

    //Clear message stream
    bzero(message, sizeof(message));

    //Read in file
    READSIZE = recv(cs, message, 2000, 0);
    
    //If the file pointer is not empty for some reason
    if(filePointer != NULL)
    {
        //Write the contents of the given file to the new file
        fwrite(message, sizeof(char), READSIZE, filePointer);
    }
    else //If there's an error writing the new file
    {
        perror("Error writing file");
        write(cs, "Incorrect privileges\n", strlen("Incorrect privileges\n"));
        pthread_mutex_destroy(&thread_lock);
        return 0;
    }

    //Close the file
    fclose(filePointer);

    write(cs, "File transferred succesfully\n", strlen("File transferred succesfully\n"));
    pthread_mutex_destroy(&thread_lock);
    
    return 0;
    
}

int main(int argc, char *argv[])
{

    int s; //Socket Descriptor 
    int cs; //Client socket
    int connSize; //Size of struct
    int READSIZE; //Size of sockaddr_in for client conn
    FILE *filePointer; //File pointer

    struct sockaddr_in server , client;
    char message [500];

    //Create a socket
    s = socket(AF_INET, SOCK_STREAM, 0);

    //Check that socket was created
    if (s == -1)
    {
        printf("Could not create socket!");
    }
    else
    {
        printf("Socket created!\n");
    }

    //Initialise socket once created
    server.sin_port = htons(8081); //Set protocol for communication
    server.sin_family = AF_INET; //Use IPv4
    server.sin_addr.s_addr = INADDR_ANY; //Bind socket to all local interfaces

    //Bind the config just set, to the socket just created
    if( bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Problem binding socket\n");
        return 1;
    }
    else
    {
        printf("Bind successful\n");
    }

    //Listen for connection
    listen(s,3);

    //Accept incoming connection
    printf("Waiting for connection from client\n");
    connSize = sizeof(struct sockaddr_in);

    
    int pid;
    pthread_t thread_id;

    pthread_mutex_init(&thread_lock, NULL);


    //Read data from the client
    while(1)
    {
        cs = accept(s, (struct sockaddr *)&client, (socklen_t*)&connSize);

        //If error establishing connection
        if(cs < 0)
        {
            perror("Can't establish connection");
            return 1;
        }
        else
        {
            printf("Connection from client accepted!\n");

            if(pthread_create(&thread_id, NULL, newConnection, (void*) &cs) < 0)
            {
                perror("Thread creation error");
                return 1;
            }
            else
            {
                printf("Thread created\n\n");
            }
        }
        
    }

    pthread_mutex_destroy(&thread_lock);

    return 0;
}