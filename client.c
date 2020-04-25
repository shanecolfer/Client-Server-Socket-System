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

    int uid;

    

    //Create socket
    SID = socket(AF_INET, SOCK_STREAM, 0);

    if(SID == -1)
    {
        printf("Error creating socket\n");
    }
    else
    {
        printf("Socket created!\n");
    }

    //Set socket variables
    server.sin_port = htons(8081); //Port to connect to
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //Server IP (local)
    server.sin_family = AF_INET; //IPv4

    //Connect to server
    if(c = connect(SID, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        return 1;
    }
    else
    {
        printf("Connected to server successfully!\n");
    }

    //Communicate with server (file transfer loop)
    while(1)
    {

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

        //Directory names
        char root[] = "/var/www/html/dev/";
        char sales[] = "/var/www/html/dev/sales/";
        char promotions[] = "/var/www/html/dev/promotions/";
        char offers[] = "/var/www/html/dev/offers/";
        char marketing[] = "/var/www/html/dev/marketing/"; 

        //Read in choice (file destination)
        scanf("%d", &choice);

        //Switch for choice (file destination)
        switch(choice)
        {
            case 1 : 
                strcat(root, fileName);
                strcpy(fullFilePath, root);
                break;
            case 2 : 
                strcat(sales, fileName);
                strcpy(fullFilePath, sales);
                break;
            case 3 : 
                strcat(promotions, fileName);
                strcpy(fullFilePath, promotions);
                break;
            case 4 :
                strcat(offers, fileName);
                strcpy(fullFilePath, offers);
                break;
            case 5 : 
                strcat(marketing, fileName);
                strcpy(fullFilePath, marketing);
                break;
            case 6 : 
                printf("Exit chosen");
                break;
            default :
                printf("Invalid Response");
        }

        if(choice == 6)
        {
            break;
        }
        else
        {
            //Open file specified
            filePointer = fopen(fileName, "r");

            //If file can't be opened
            if(filePointer == NULL)
            {
                perror("Cannot open file");
                return 1;
            }


            uid = getuid();
            int converted_uid = htonl(uid);
            send(SID, &uid, sizeof(uid), 0);
            printf("Sent uid to server: ");
            printf("%d", uid);
            printf("\n");
            recv(SID, &serverMessage, 500, 0);
            printf("Received response: ");
            printf(serverMessage);
            printf("\n");
            //Wipe server response
            bzero(serverMessage, sizeof(serverMessage));

            //Client message = full file path
            strcpy(clientMessage, fullFilePath);

            //Send desired file path to server
            send(SID, clientMessage, sizeof(clientMessage), 0);

            printf("Path sent to server : ");
            printf(clientMessage);
            printf("\n");

            //Wipe client message
            bzero(clientMessage, sizeof(clientMessage));

            //Handle server response
            recv(SID, serverMessage, 500, 0);
            printf("Received response: ");
            printf(serverMessage);
            printf("\n");
            
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
            printf("\n\n");
            
            //Wipe server response
            bzero(serverMessage, sizeof(serverMessage));

            //Wipe filename
            bzero(fileName, sizeof(fileName));

            //Wipe client message
            bzero(clientMessage, sizeof(clientMessage));


            //Close the file pointer
            fclose(filePointer);
        }
        
        
        

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