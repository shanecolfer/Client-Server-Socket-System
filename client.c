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
    int c;

    //Initialise file variables
    FILE *filePointer;
    char fileName[300];
    char fullFilePath[300];

    //Choice variable
    int choice;

    //Directory names
    char root[] = "/var/www/html/";
    char sales[] = "/var/www/html/sales";
    char promotions[] = "/var/www/html/promotions";
    char offers[] = "/var/www/html/offers";
    char marketing[] = "var/www/html/marketing";

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
    if(c = connect(SID, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed");
        return 1;
    }
    else
    {
        printf("Connected to server successfully!");
    }


    //Read in name of file
    printf("Enter the filename\n");
    scanf("%s", fileName);
    printf(fileName);

    //Display options to user for transfer location
    printf("\nEnter a location to transfer file: ");
    printf("\n 1: Root of intranet website");
    printf("\n 2: Sales");
    printf("\n 3: Promotions");
    printf("\n 4: Offers");
    printf("\n 5: Marketing");
    printf("\n 6: Exit\n");

    //Communicate with server
    while(1)
    {
        //Read in choice (file destination)
        scanf("%d", &choice);

        //Switch for choice (file destination)
        switch(choice)
        {
            case 1 : 
                strcat(root, fileName);
                break;
            default :
                printf("Invalid Response");
                break;
        }
        
        //Open file specified
        filePointer = fopen(fileName, "r");

        //If file can't be opened
        if(filePointer == NULL)
        {
            perror("Cannot open file");
            return 1;
        }

        //Client message = full file path
        strcpy(clientMessage, root);

        //Send desired file path to server
        send(SID, clientMessage, sizeof(clientMessage), 0);

        //Wipe client message
        bzero(clientMessage, sizeof(clientMessage));

        //Handle server response
        recv(SID, serverMessage, 500, 0);
        printf("Received response: ");
        printf(serverMessage);
        
        //Wipe server response
        bzero(serverMessage, sizeof(serverMessage));

        //Send file to server
        while(( c = fread(clientMessage, sizeof(char), sizeof(clientMessage), filePointer)) > 0)
        {
            //Send
            send(SID, clientMessage, c, 0);
            //Wipe
            bzero(clientMessage, sizeof(clientMessage));
        }

        //Handle server response
        recv(SID, serverMessage, 500, 0);
        printf("Received response: ");
        printf(serverMessage);
        
        //Wipe server response
        bzero(serverMessage, sizeof(serverMessage));


        //Close the file pointer
        fclose(filePointer);
        return 0;

        /* GRAVEYARD
        //Copy file to client message variable
        strcpy(clientMessage, fileName);

        //Send file to server
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
        */
    }

    printf("Down below");

    close(SID);
    return 0;
    

}