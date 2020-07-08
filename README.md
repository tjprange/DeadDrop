# DeadDrop
Program 4 – CS 344
Overview
In this assignment, you will be creating three small programs that encrypt and decrypt information using a one-time pad-like system, and a single script to compile the three programs. I believe that you will find the topic quite fascinating: one of your challenges will be to pull yourself away from the stories of real-world espionage and tradecraft that have used the techniques you will be implementing.

These programs serve as a capstone to what you have been learning in this course, and will combine the multi-processing code you have been learning with socket-based inter-process communication. Your programs will also be accessible from the command line using standard UNIX features like input/output redirection, and job control. Finally, you will write a short compilation script.

Specifications
All execution, compiling, and testing of this program should ONLY be done in the bash prompt on our class server!

Use the following link as your primary reference on One-Time Pads (OTP):

http://en.wikipedia.org/wiki/One-time_pad (Links to an external site.) (Links to an external site.)

The following definitions will be important:

Plaintext is the term for the information that you wish to encrypt and protect. It is human readable.

Ciphertext is the term for the plaintext after it has been encrypted by your programs. Ciphertext is not human-readable, and in fact cannot be cracked, if the OTP system is used correctly.

A Key is the random sequence of characters that will be used to convert Plaintext to Ciphertext, and back again. It must not be re-used, or else the encryption is in danger of being broken.

The following excerpt from this Wikipedia article was captured on 2/21/2015:

“Suppose Alice wishes to send the message "HELLO" to Bob. Assume two pads of paper containing identical random sequences of letters were somehow previously produced and securely issued to both. Alice chooses the appropriate unused page from the pad. The way to do this is normally arranged for in advance, as for instance 'use the 12th sheet on 1 May', or 'use the next available sheet for the next message'.

The material on the selected sheet is the key for this message. Each letter from the pad will be combined in a predetermined way with one letter of the message. (It is common, but not required, to assign each letter a numerical value, e.g., "A" is 0, "B" is 1, and so on.)

In this example, the technique is to combine the key and the message using modular addition. The numerical values of corresponding message and key letters are added together, modulo 26. So, if key material begins with "XMCKL" and the message is "HELLO", then the coding would be done as follows:

      H       E       L       L       O  message
   7 (H)   4 (E)  11 (L)  11 (L)  14 (O) message
+ 23 (X)  12 (M)   2 (C)  10 (K)  11 (L) key
= 30      16      13      21      25     message + key
=  4 (E)  16 (Q)  13 (N)  21 (V)  25 (Z) message + key (mod 26)
      E       Q       N       V       Z  → ciphertext
If a number is larger than 26, then the remainder, after subtraction of 26, is taken [as the result]. This simply means that if the computations "go past" Z, the sequence starts again at A.

The ciphertext to be sent to Bob is thus "EQNVZ". Bob uses the matching key page and the same process, but in reverse, to obtain the plaintext. Here the key is subtracted from the ciphertext, again using modular arithmetic:

       E       Q       N       V       Z  ciphertext
    4 (E)  16 (Q)  13 (N)  21 (V)  25 (Z) ciphertext
-  23 (X)  12 (M)   2 (C)  10 (K)  11 (L) key
= -19       4      11      11      14     ciphertext – key
=   7 (H)   4 (E)  11 (L)  11 (L)  14 (O) ciphertext – key (mod 26)
       H       E       L       L       O  → message
Similar to the above, if a number is negative then 26 is added to make the number zero or higher.

Thus Bob recovers Alice's plaintext, the message "HELLO". Both Alice and Bob destroy the key sheet immediately after use, thus preventing reuse and an attack against the cipher.”

Your program will encrypt and decrypt plaintext into ciphertext, using a key, in exactly the same fashion as above, except it will be using modulo 27 operations: your 27 characters are the 26 capital letters, and the space character ( ). All 27 characters will be encrypted and decrypted as above.

To do this, you will be creating three small programs in C. One of these will function like a "daemon" (but isn’t actually a daemon), and will be accessed using network sockets. Another will use the daemon to perform work, and the last is a standalone utility.

Your programs must use the network calls we've discussed in class (send(), recv(), socket(), bind(), listen(), & accept()) to send and receive sequences of bytes for the purposes of encryption and decryption by the appropriate daemons. The whole point is to use the network, even though for testing purposes we're using the same machine: if you just open() the plaintext file from the server or the ciphertext files from the client, you'll receive 0 points on the assignment.

Here are the specifications of the three programs:

otp_d: This program will run in the background as a daemon. Upon execution, otp_d must output an error if it cannot be run due to a network error, such as the ports being unavailable. Its function is to store the encrypted data. This program will listen on a particular port/socket, assigned when it is first ran (see syntax below). When a connection is made, otp_d must call accept() to generate the socket used for actual communication, and then use a separate process to handle the rest of the transaction (see below), which will occur on the newly accepted socket.

For testing purposes, the first thing the child of otp_d must do after a connection is accepted is call sleep(2). If this is not included, you will lose points.

This child process of otp_d must first check if otp (see otp, below) is connecting in post or get mode. If otp has connected in post mode, then this child receives from otp a username and encrypted message via the communication socket (not the original listen socket). The otp_d child will then write the encrypted message to a file and print the path to this file (from the directory otp_d was started in) to stdout. If otp_d should crash or terminate for any reason, it should still be able to retrieve the messages after being restarted.
If otp has connected in get mode, then the child process of otp_d will receive from otp only a username. The child will retrieve the contents of the oldest file for this user and send them to otp. Finally, it should delete the ciphertext file.

Your version of otp_d must support up to (and at most) five concurrent socket connections running at the same time; this is different than the number of processes that could queue up on your listening socket (which is specified in the second parameter of the listen() call). Again, only in the child process will the actual storage and retrieval take place, and the ciphertext be written back: the original server daemon process continues listening for new connections, not processing any requests.

In terms of creating that child process as described above, you may either create with fork() a new process every time a connection is made, or set up a pool of five processes at the beginning of the program, before connections are allowed, to handle your encryption tasks. As above, your system must be able to do five separate posts/gets at once, using either method you choose.

Use this syntax for otp_d:

otp_d listening_port
listening_port is the port that otp_d should listen on. You will always start otp_d in the background, as follows (the port 57171 is just an example; yours should be able to use any port):

$ otp_d 57171 &
In all error situations, this program must output errors to stderr as appropriate (see grading script below for details), but should not crash or otherwise exit, unless the errors happen when the program is starting up (i.e. are part of the networking start up protocols like bind()). If given bad input, once running, otp_d should recognize the bad input, report an error to stderr, and continue to run. Generally speaking, though, this daemon shouldn't receive bad input, since that should be discovered and handled in the client first. All error text must be output to stderr.

This program, and the other network programs, should use "localhost" as the target IP address/host. This makes them use the actual computer they all share as the target for the networking connections.

otp: This program connects to otp_d and asks it to store or retrieve messages for a given user. It has two modes: post and get. The syntax of otp in post mode is as follows:

otp post user plaintext key port
In this syntax, user is the name you want otp_d to associate with the encrypted message. plaintext is the name of a file in the current directory that contains the plaintext you wish to encrypt. Similarly, key contains the encryption key you wish to use to encrypt the text. Finally, port is the port that otp should attempt to connect to otp_d on. In post mode, otp will first encrypt plaintext using key, using the method described above. It will then send user and the encrypted message to otp_d, where otp_d stores it as described above in otp_d.

In get mode, the syntax of otp is as follows:

otp get user key port
In this syntax, user is the name you want otp_d to retrieve an encrypted message for. key contains the decryption key you wish to use to decrypt the retrieved ciphertext. Finally, port is the port that otp should attempt to connect to otp_d on. In get mode, otp will send a request for a message for user. otp will then use key to decrypt the message and print the decrypted message to stdout. If user does not have any message, otp should report an error stderr.

Thus, otp can be launched in get mode in any of the following methods, and should send its output appropriately:

$ otp get Ben mykey 57171
$ otp get Ben mykey 57171 > myplaintext
$ otp get Ben mykey 57171 > myplaintext &
If otp receives key or plaintext files with ANY bad characters in them, or the key file is shorter than the plaintext, then it should terminate, send appropriate error text to stderr, and set the exit value to 1.

If otp cannot connect to the otp_d server, for any reason, it should report this error to stderr with the attempted port, and set the exit value to 2. Otherwise, upon successfully running and terminating, otp should set the exit value to 0.

Again, any and all error text must be output to stderr (not into the plaintext or ciphertext files).

keygen: This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard UNIX randomization methods. Do not create spaces every five characters, as has been historically done. Note that you specifically do not have to do any fancy random number generation: we’re not looking for cryptographically secure random number generation! rand() is just fine. The last character keygen outputs should be a newline. All error text must be output to stderr, if any.

The syntax for keygen is as follows:

keygen keylength
Where keylength is the length of the key file in characters. keygen outputs to stdout. Here is an example run, which redirects stdout to a key file of 256 characters called “mykey” (note that mykey is 257 characters long because of the newline):

$ keygen 256 > mykey
Files and Scripts

You are provided with 5 plaintext files to use (one, two, three, four, five). The grading will use these specific files; do not feel like you have to create others.

You are also provided with a grading script ("p4gradingscript") that you can run to test your software. If it passes the tests in the script, and has sufficient commenting, it will receive full points (see below). EVERY TIME you run this script, change the port numbers you use! Otherwise, because UNIX may not let go of your ports immediately, your successive runs may fail!

Finally, you will be required to write a compilation script (see below) that compiles all three of your programs, allowing you to use whatever C code and methods you desire. This will ease grading. Note that only C will be allowed, no C++ or any other language (Python, Perl, awk, etc.).

Example

Here is an example of usage, if you were testing your code from the command line:

$ cat plaintext1
THE RED GOOSE FLIES AT MIDNIGHT STOP
$ otp_d 57171 &
$ keygen 10
EONHQCKQ I
$ keygen 10 > mykey
$ cat mykey
VAONWOYVXP
$ keygen 10 > myshortkey
$ otp post Ben plaintext1 myshortkey 57171
Error: key ‘myshortkey’ is too short
$ echo $?
1
$ keygen 1024 > mykey
$ otp post Ben plaintext1 mykey 57171
path/to/ciphertext/file # otp_d prints this. Yours should print the actual path.
$ cat ciphertextpath
WANAWTRLFTH RAAQGZSOHCTYS JDBEGYZQDQ
$ keygen 1024 > mykey2
$ otp get Ben mykey 57171 > plaintext1_a
$ otp post Ben plaintext1 mykey 57171
path/to/ciphertext/file # otp_d prints this. It is the path to the ciphertext file it just stored.
$ otp get Ben mykey2 57171 > plaintext1_b
$ cat plaintext1_a
THE RED GOOSE FLIES AT MIDNIGHT STOP
$ cat plaintext1_b
WSXFHCJAEISWQRNO L ZAGDIAUAL IGGTKBW
$ cmp plaintext1 plaintext1_a
$ echo $?
0
$ cmp plaintext1 plaintext1_b
plaintext1 plaintext1_b differ: byte 1, line 1
$ echo $?
1
$ otp post Ben plaintext5 mykey 57171
otp error: input contains bad characters
$ echo $?
1
$
Compilation Script

You must also write a short bash shell script called “compileall” that merely compiles your three programs. For example, the first two lines might be:

#!/bin/bash
gcc -o otp_d otp_d.c
…
This script will be used to compile your software, and must successfully run on our class server. The compilation must create all three programs, in the same directory as “compileall”, for immediate use by the grading script, which is named “p4gradingscript”.
