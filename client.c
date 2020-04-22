#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    //Initialise client variables
    int SID;
    struct sockaddr_in server;
    char clientMessage[500];
    char serverMessage[500];

    //Create socket
    SID = socket(AF_INET, SOCK_STREAM, 0);

    if(SID == -1)
    {
        printf("Error creating socket");
    }
    else
    {
        printf("Socket created!");
    }

    //Set socket variables
    server.sin_port = htons(8081); //Port to connect to
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //Server IP (local)
    server.sin_family = AF_INET; //IPv4

    //Connect to server
    if(connect(SID, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed");
        return 1;
    }
    else
    {
        printf("Connected to server successfully!");
    }

    //Communicate with server
    while(1)
    {
        printf("\nEnter a message: ");
        scanf("%s", clientMessage);

        //Send message to server
        if(send(SID, clientMessage, strlen(clientMessage), 0) < 0)
        {
            printf("Send Failed");
            return 1;
        }

        //Receive message reply from server
        if(recv(SID, serverMessage, 500, 0) < 0)
        {
            printf("Error receiving reply from server");
            break;
        }

        printf("\nServer Reply: ");
        printf(serverMessage);
    }

    close(SID);
    return 0;
    

}