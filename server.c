#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <pthread.h>
#include <sys/fsuid.h>
#include <grp.h>
#include <pwd.h>

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
    uid_t uid = getuid();
    uid_t gid = getgid();
    uid_t euid = geteuid();
    uid_t geid = getegid();

    gid_t supp_groups[] = {};

    //Make sure that we're running with root privilege
    uid = 0;

    setreuid(uid, uid);
    setregid(uid, uid);
    seteuid(uid);
    setegid(gid);

    int j, ngroups;
    gid_t *groups;
    struct group *gr;

    ngroups = 10;
    groups = malloc(ngroups * sizeof (gid_t));

    char message [500];
    int cs = *(int*)connectionsocket;
    int READSIZE;
    FILE *filePointer; //File pointer
    FILE *filePointerTemp;

    //Read UID of user
    READSIZE = read(cs, &received_uid, sizeof(received_uid));
    received_uid = ntohl(received_uid);
    uid_t new_uid = received_uid;
    printf("UID received: %d\n", new_uid);

    write(cs, "UID Received\n", strlen("UID Received\n"));

    //Read GID of user
    READSIZE = read(cs, &received_gid, sizeof(received_gid));
    received_gid = ntohl(received_gid);
    gid_t new_gid = received_gid; 
    printf("GID received: %d\n", new_gid);

    write(cs, "GID Received\n", strlen("GID Received\n"));
    
    //Get user name from password file struct
    struct passwd *pw;
    pw = getpwuid(new_uid);

    //Get groups according to user name
    getgrouplist(pw->pw_name, getuid(), groups, &ngroups); //HARD CODED GET THE USER NAME

    //Put groups into array
    for(j = 0; j < ngroups; j++)
    {
        supp_groups[j] = groups[j];
    }

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

    char fileName[500]; //Declare filename variables
    strcpy(fileName, message); //Assign filename the contents of messge
    printf("File location request: %s\n", fileName); //Printing
    //printf("User ID of program after: ");
    //uid = getuid();
    //printf("%d", uid);
    write(cs, "File location request received\n", strlen("File location request received\n"));

    //Write to a temp file
    char tempDir[500] = "/var/www/html/dev/temps/tmp.tmp";


    //Filepointer temp = location of temp file 
    //filePointerTemp = fopen(tempDir, "w");

    
    //Create file pointer to temp location
    filePointer = fopen(tempDir, "w");

    //Clear message stream
    bzero(message, sizeof(message));

    //Read in file
    READSIZE = recv(cs, message, 2000, 0);
    
    //If the file pointer is not empty for some reason
    if(filePointer != NULL)
    {
        //Write the contents of the given file to the new file
        fwrite(message, sizeof(char), READSIZE, filePointer);
        printf("Temp file written\n");
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


    char cp [500] = "cp ";
    //char fileName1[500] = "/var/www/html/dev/offers/test.txt";
    strcat(cp, tempDir);
    strcat(cp, " ");
    strcat(cp, fileName);

    //printf("Full Command: %s\n", cp);
    //Set groups
    setgroups(10, supp_groups);

    //Set IDs
    
    if(setreuid(new_uid, uid) < 0)
    {
        printf("%s: cannot change euid\n");
    }

    if(setregid(new_gid, gid) < 0)
    {
        printf("%s: cannot change guid\n");
    }

    if(seteuid(new_uid) < 0)
    {
        printf("%s: cannot change euid\n");
    }

    if(setegid(new_uid) < 0)
    {
        printf("%s: cannot change euid\n");
    }

    printf("User ID: %d\n", getuid());
	printf("Group ID: %d\n", getgid());
	printf("E User ID: %d\n", geteuid());
	printf("E Group ID: %d\n", getegid());

    //Carry out system command and check for error
    if(system(cp) == 0)
    {
        printf("File written succesfully\n");
        write(cs, "File transfer successful!\n", strlen("File transfer successful!\n"));
    }
    else
    {
        write(cs, "Incorrect privilges\n", strlen("Incorrect priviliges\n"));
    }

    //Swap back to root privilege
    uid = 0;
    gid = 0;

    setregid(gid, gid);
    setreuid(uid, uid);
    setegid(gid);
    seteuid(uid);
    

    printf("User ID: %d\n", getuid());
	printf("Group ID: %d\n", getgid());
	printf("E User ID: %d\n", geteuid());
	printf("E Group ID: %d\n", getegid());
    

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