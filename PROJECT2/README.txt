Project2-CS-551

Developers:
1.Goutham Kannan A20361163  
2.Imran Ali Usmani - A20362180

The folder contains the following files:
1. misc.c : contains code for the MailBox implementation and its functions
2. table.c : contains table of all the system calls to Process Management Server
3. proto.h : Header file containing the declaration of the system call and the function implementation 
4. Callnr.h : contains the list of system calls and a number associated to them
5. sender.c: An user process that can send messages to multiple receivers
6. receiver.c : An user process than can receive their corresponding messages
7. receiver_with_life_lock.c : An user process than can receive their corresponding messages with a life-lock condition 
8. clear.c : user process to clear the mailbox
9. setup.c : user process to setup the mailbox
10. mailbox.h : Provides a better abstraction to the end users as they can use easy to understand calls like send()/receive() instead of complex _SYSCALL[…] call.
11. Design_Doc : A design document.

For detailed information about the implementation, Please refer to the Design Document. 

Steps to build:
The folder contains the user programs that use the implemented MailBox functions. If you want to make changes and rebuild
again, use following commands:
1. # make receiver
2. # make sender
3. # make receiver_with_life_lock
4. # make clear
5. # make setup 

	
Steps to test:

	1. Steps to send message from a user program:
	The receiver user programs are implemented to use the command line arguments as inputs,
	Syntax:
	> ./sender <Message_to_be_sent> <receiver_list>
	Eg. 
	> ./sender Message1 1
	> ./sender Message2 1 2 3

	2. Steps to receive the message in a user program
	The receiver user program is implemented to use get the reciever name list as command line input.
	Syntax:
	>./receiver <receiver_list>
	
	Eg. 
	> ./receiver 1
	> Message recieved is Message1 
	> ./receiver 1 2
	> Message recieved is Message2
	> Message recieved is Message2
	
	3. Steps to clear the Mailbox
	> ./clear
	
	4. Steps to setup the mailbox
	> ./setup
	

