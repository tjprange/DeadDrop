/*
Thomas Prange
CS 344
Dead Drop - keygen.c
5/26/2020
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* This function will return a randomly chosen value from keyValues, which is an 
array of A-Z and " " characters. */
char getRandomChar(){
    char keyValues[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                        'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                        'U', 'V', 'W', 'X', 'Y', 'Z',  ' '
                       };

    int random = rand() % 27;
    return keyValues[random];
}

void main(int argc, char *argv[]){
    srand(time(0));     
                        
    if (argc != 2){
        fprintf(stderr, "USAGE: keygen keyLength\n");
        exit(1);
    }
    
    // get size from second arg
    int size = atoi(argv[1]);

    // if invalid arg[1] value, exit
    if (size == 0){
        fprintf(stderr, "Invalid argument, argv[1] must be an integer value!\n");
        exit(1);
    }
    
    // create array of size size+1. The +1 is for the '\n' char. 
    char key[size+1];

    // fill key array with random values
    int i;
    for (i = 0; i < size; i++){
       key[i] = getRandomChar();
    }
    
    // set last char to newline
    key[size] = '\n';

    // print the string
    printf("%s", key);
}