#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <dirent.h>

typedef enum {false, true} bool;
bool post;

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

void getOldestFile(char oldestFile[],char *name){ //newestDirName
    
    memset(oldestFile, '\0', sizeof(oldestFile));
    int oldesetDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32];
    strcpy(targetDirPrefix, name);

    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir

    dirToCheck = opendir("."); // Open up the directory this program was run in

    if (dirToCheck > 0) // Make sure the current directory could be opened
    {
        
        while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
            
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
            {
                
                memset(oldestFile, '\0', sizeof(oldestFile));
                strcpy(oldestFile, fileInDir->d_name);
                oldesetDirTime = (int)dirAttributes.st_mtime;
                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

                if ((int)dirAttributes.st_mtime < oldesetDirTime) // If this time is bigger
                {
                    oldesetDirTime = (int)dirAttributes.st_mtime;
                    memset(oldestFile, '\0', sizeof(oldestFile));
                    strcpy(oldestFile, fileInDir->d_name);
                }
            }
        }
    }

    closedir(dirToCheck); // Close the directory we opened
}

int main(int argc, char *argv[])
{
        int listenSocketFD, establishedConnectionFD, portNumber;
        socklen_t sizeOfClientInfo;
        struct sockaddr_in serverAddress, clientAddress;

        if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

        // Set up the address struct for this process (the server)
        memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
        portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
        serverAddress.sin_family = AF_INET; // Create a network-capable socket
        serverAddress.sin_port = htons(portNumber); // Store the port number
        serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

        // Set up the socket
        listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
        if (listenSocketFD < 0) error("ERROR opening socket");

        // Enable the socket to begin listening
        if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
                error("ERROR on binding");
        listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
        while(1){
                // Accept a connection, blocking if one is not available until one connects
                sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
                establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
                if (establishedConnectionFD < 0) error("ERROR on accept");


                int r;
                char completeMessage[100000], readBuffer[10];
                memset(completeMessage, '\0', sizeof(completeMessage)); // Clear the buffer
                while (strstr(completeMessage, "@@") == NULL) // As long as we haven't found the terminal...
                {
                        memset(readBuffer, '\0', sizeof(readBuffer)); // Clear the buffer
                        r = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer) - 1, 0); // Get the next chunk
                        strcat(completeMessage, readBuffer); // Add that chunk to what we have so far
                        if (r == -1) { printf("r == -1\n"); break; } // Check for errors
                        if (r == 0) { printf("r == 0\n"); break; }
                }
                int terminalLocation = strstr(completeMessage, "@@") - completeMessage; // Where is the terminal
                completeMessage[terminalLocation] = '\0'; // End the string early to wipe out the terminal

                char *name;
                char encryptedMessage[100000];
                int counter = 0;

                char *token;
                // Set post mode to true or false
                if (completeMessage[0] == '0'){
                        post = true;
                }
                else {
                        post = false;
                }

                // pull the name from the complete message
                token = strtok(completeMessage, " ");
                while (token != NULL){
                        if (counter == 1){
                                strcpy(name, token);
                                break;
                        }
                        counter++;
                        token = strtok(NULL, " ");
                }

                int size = strlen(name) + 2;

                // remove the spaces and the name from completeMessage and copy into encrypted message
                strcpy(encryptedMessage, &completeMessage[size+1]);

                pid_t spawnPid = -5;
                int childExitStatus = -5;
                // if post is true, then fork a child, create a file and put the message into the file
                char fileName[50];
                spawnPid = fork();
                switch (spawnPid){
                        case -1: // error
                                perror("Hull Breach!"); exit(1); break;
                        case 0: // child
                                sleep(2);
                                // If the mode is post mode, then the encrypted key will be written to a 
                                // file in the form of name.ciphertext.pid and the file path is printed
                                if(post){
                                        char path[100];
                                        int pid = getpid();                                        
                                        sprintf(fileName, "%s.cyphertext.%d", name, pid);
                                        int file_descriptor = open(fileName, O_WRONLY | O_CREAT, 0600);
                                        write(file_descriptor, encryptedMessage, strlen(encryptedMessage) * sizeof(char));
                                        char *filePath = realpath(fileName, path);
                                        printf("%s\n", filePath);
                                        fflush(stdout);	
                                        exit(0);
                                }
                                else { // getmode
                                    char oldestFile[256];
                                    getOldestFile(oldestFile, name);
                                    FILE *fp;
                                    // open key for reading
                                    fp = fopen(oldestFile, "r");
                                    if (fp == NULL){
                                        char message[] = "User does not exist";
                                        send(establishedConnectionFD, message, strlen(message), 0);  
                                        send(establishedConnectionFD, "@@", strlen("@@"), 0);    
                                        exit(1);
                                    }
                                    char eMessage[100000];
                                    fgets(eMessage, 100000, fp);
                                    send(establishedConnectionFD, eMessage, strlen(eMessage), 0); 
                                    send(establishedConnectionFD, "@@", strlen("@@"), 0);   
                                    fclose(fp);
                                    remove(oldestFile);
                                    exit(0);
                                }
                                break;
                        default:  // parent              
                                //waitpid(spawnPid, &childExitStatus, 0);                  
                                waitpid(spawnPid, &childExitStatus, WNOHANG);
                                break;
                }
        }
        close(listenSocketFD); // Close the listening socket
        return 0; 
}