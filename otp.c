#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// Global variables
typedef enum {false, true} bool;
bool get;           // holds the string from argv[1]
char *user;         // holds the string from argv[2]
char *plain;        // holds the string from argv[3]
char *key;          // holds the string from argv[4]
char *keyText;      // string to hold chars of key file
char *plainText;    // string to hold chars of plaintext file
char encryptedMessage[100000]; // holds encrypted message
char decryptedMessage[100000]; // holds decrypted message
int port;

int keyChars = 0; // keyChars hold the number of chars in the key
int textChars = 0; // holds the # of chars in the plaintext file

// array to hold valid characters
char keyValues[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                     'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                     'U', 'V', 'W', 'X', 'Y', 'Z',  ' '
                    };

/* This function will check the number of arguments supplied. It will set the "get" flag as appropriate or if the args supplied
aren't sufficient, then print error message */ 
void checkArgs(int argc){
    if(argc == 5){
        get = true;
    }
    else if(argc == 6){
        get = false;
    }
    else {
        fprintf(stderr,"USAGE (post): otp post user plantext key port\n");
        fprintf(stderr, "OR\n");
        fprintf(stderr, "USAGE (get): otp get user key port\n");
        exit(1); 
    }
}

/* This function will check that the length of the key is sufficient */
void checkKeyLength(){
    FILE *keyFP, *textFP;
    char ch;

    // open key for reading
    keyFP = fopen(key, "r");
    // file doesn't exist
    if (keyFP == NULL){
        fprintf(stderr, "Bad key file\n");
        exit(1);
    }
    // count chars in key
    for ( ch = getc(keyFP); ch != EOF; ch = getc(keyFP)){
        keyChars++;
    }
    fclose(keyFP);

    // open plaintext for reading
    textFP = fopen(plain, "r");
    // file doesn't exist
    if (textFP == NULL){
        fprintf(stderr, "Bad plaintext file\n");
        exit(1);
    }
    // count chars in plaintext
    for ( ch = getc(textFP); ch != EOF; ch = getc(textFP)){
        textChars++;
    }
    fclose(textFP);
    
    // exit if key is insufficient size
    if (keyChars < textChars ){
        fprintf(stderr, "Error: '%s' is too short\n", key);
        exit(1);
    }
}

/* This function will check for invalid characters and exit if they are encountered */
void checkValidKey(){
    FILE *textFP;
    textFP = fopen(plain, "r");
    char ch;
    // error opening fle
    if (textFP == NULL){
        fprintf(stderr, "Bad plaintext file\n");
        exit(1);
    }

    // check for bad characters
    for ( ch = getc(textFP); ch != EOF; ch = getc(textFP)){
        // If the chars arent uppercase, " " or "\n"
        if((ch < 65 || ch > 90) && ch != 32 && ch != 10){
            fprintf(stderr, "Error: plaintext contains bad characters\n");
            exit(1);
        }
    }
    fclose(textFP);
}

/* This will copy keyChars number of characters into the keyText string */
void copyKey(){
    FILE *keyFP;
    char ch;

    // open key for reading
    keyFP = fopen(key, "r");

    // allocate string
    keyText = (char *)malloc(sizeof(char) * textChars);

    int i = 0;
    ch = getc(keyFP);
    // copy key into keyText string
    while (ch != EOF){ 
        if (i == textChars) { break; }       
        keyText[i] = ch;        
        i++;
        ch = getc(keyFP);
    }
    fclose(keyFP);
    keyText[textChars] = '\0';    
}

/* This function will return the index of ch in keyValues */
int getIndex(char *ch){
    int i;
    int index = 0;
    int numCharacters = 27;
    for(i = 0; i < numCharacters; i++){
        if(keyValues[i] == *ch){
            index = i;
        }
    }
    return index;
}

/* This function will encode the plaintext message using the key */
void encryptMessage(){
    int i, index;

    FILE *textFP;
    textFP = fopen(plain, "r");
    char ch;
    ch = getc(textFP);

    // This will add the 0-27 from ch in the plaintext file to the value of the 
    // character in the keyText file then mod 27
    for (i = 0; i < textChars; i++){
        index = getIndex(&ch) + getIndex(&keyText[i]);
        index %= 27;
        encryptedMessage[i] = keyValues[index];
        ch = getc(textFP);              
    }
    // Null terminate the string
    encryptedMessage[keyChars-1] = '\0';    
    fclose(textFP);
}

/* This function will decrypt a message using the key */
void decryptMessage(){
    int i, index, a, b;
    FILE *fp;
    // open key for rfopen(key, "r");eading
    fp = fopen(key, "r");
    if (fp == NULL){
    fprintf(stderr, "Bad key file\n");
        exit(1);
    }

    char keyText[100000];
    fgets(keyText, 100000, fp);
    
    textChars = strlen(encryptedMessage);
    // Loop through chars in encrytpedMessage and keyText, then convert 
    // the encrypted message using the key and modulus operator 
    for (i = 0; i < textChars; i++){        
        a = getIndex(&encryptedMessage[i]);
        b = getIndex(&keyText[i]);
        index = a - b;
        if (index < 0){
            index += 27;
        }
        index %= 27;
        decryptedMessage[i] = keyValues[index];
    }
    decryptedMessage[textChars-1] = '\0';
    printf("%s\n", decryptedMessage);
}

/* This function will send the mode type, username and encrypted message to opt_d */
void post(){
    int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
    
    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = port; // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);// Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) error("CLIENT: ERROR opening socket");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to addy
        error("CLIENT: ERROR connecting");

    // Send post mode to server
    char mode[] = "0";
    charsWritten = send(socketFD, mode, strlen(mode), 0); // Write to the server
    send(socketFD, " ", strlen(" "), 0);

    // Send user to server
    charsWritten = send(socketFD, user, strlen(user), 0); // Write to the server
    send(socketFD, " ", strlen(" "), 0);
    if (charsWritten < strlen(user)) printf("CLIENT: WARNING: Not all data written to socket!\n");

    // Send encryptedMessage to server
    charsWritten = send(socketFD, encryptedMessage, strlen(encryptedMessage), 0); // Write to the server
    // @@ will dictate string termination
    send(socketFD, "@@", strlen("@@"), 0);

    close(socketFD);
}


/* This function will process the operations for post */
void postOperations(){
    checkKeyLength();
    checkValidKey();
    copyKey();    
    encryptMessage();
    post();   
    
}

/* This function will retrieve the message from the server */
void getMessage(){
    
    int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
    char buffer[100000];
    
    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = port; // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);// Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) error("CLIENT: ERROR opening socket");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to addy
        error("CLIENT: ERROR connecting");

    // Send get mode to server
    char mode[] = "1";
    charsWritten = send(socketFD, mode, strlen(mode), 0); // Write to the server
    send(socketFD, " ", strlen(" "), 0);

    // Send user to server
    charsWritten = send(socketFD, user, strlen(user), 0); // Write to the server
    send(socketFD, " ", strlen(" "), 0);
    if (charsWritten < strlen(user)) printf("CLIENT: WARNING: Not all data written to socket!\n");

    send(socketFD, "@@", strlen("@@"), 0);

    memset(buffer, '\0', sizeof(encryptedMessage)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, encryptedMessage, sizeof(encryptedMessage) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");

    int r;
    char readBuffer[10];
    while (strstr(encryptedMessage, "@@") == NULL) // As long as we haven't found the terminal...
    {
            memset(readBuffer, '\0', sizeof(readBuffer)); // Clear the buffer
            r = recv(socketFD, readBuffer, sizeof(readBuffer) - 1, 0); // Get the next chunk
            strcat(encryptedMessage, readBuffer); // Add that chunk to what we have so far
            if (r == -1) { printf("r == -1\n"); break; } // Check for errors
            if (r == 0) { printf("r == 0\n"); break; }
    }
    int terminalLocation = strstr(encryptedMessage, "@@") - encryptedMessage; // Where is the terminal
    encryptedMessage[terminalLocation] = '\0'; // End the string early to wipe out the terminal

    // Client will print this message if user does not have a file
    char *message = "User does not exist";
    if (strcmp(encryptedMessage, message) == 0){
        printf("There are no messages for %s\n", user);
        fflush(stdout);
        exit(1);
    }
	    
    close(socketFD);

}

/* This function will process the operations for get */
void getOperations(){
    getMessage();
    decryptMessage();
}

/* This function will assign the user, key, port and plaintext variables as specified by the args */
void argsToVars(int argc, char *argv[]){    
    if(argc == 5){
        user = argv[2];
        key = argv[3];
        port = atoi(argv[4]);
    }
    else {
        user = argv[2];
        plain = argv[3];
        key = argv[4];
        port = atoi(argv[5]);
    }
}

int main(int argc, char *argv[]){
    // check number of args and set get flag
    checkArgs(argc);
    // assign args to variables
    argsToVars(argc, argv);

    if(get){ // operations for get method
        getOperations();
        free(keyText);
    }
    else { // operations for post method        
        postOperations();
    }
    
    return 0;
}