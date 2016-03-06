Project1-CS-551

Developers :
1.Goutham Kannan A20361163  
2.Imran Ali Usmani - A20362180
3.Cheithan Bapatla A20358371

The folder contains the following files :
1. main.c : conatins code for the shell
2. .profile : contains initialization values for shell
3. Alias_list.txt: A text file to store alias info. of shell
4. my_shell : An executable shell script to execute .profile file
5. test.c : A C file, which blocks on executing. Contains an infinite loop. 
			Used for test purpose.
6. main : An executable binary of main.c file
7. test : An executable binary of test.c file
8. grep : Used internally for parsing .profile
9. ps : Used for testing '&' functionality(running process in background)
10. kill: Used for testing '&' functionality
11. Design_Doc : A design document.

Steps to build:
The folder contains the build binaries. If you want to make changes and rebuild
again, use following commands:
1. # make main
2. # make test

Steps to execute:
Run the following commands:
	# ./my_shell

Steps to configure using .profile:
1. To set block timer off, add: 
	TIMER=0
2. To set timer for "n" seconds, where n is any positive integer, add:
	TIMER=n
3. To set PATH variable, add:
	PATH=/bin:/usr/bin	
	
Steps to test:
1. To test the timer, run the following commands after starting the shell created by us. :
	> ./test 
   If the timer prompt appears, enter "y". The "./test" process should terminate and command
   prompt should appeear again
2. To test the working of ".profile". Make the following changes in ".profile"
   a. TIMER=0 
      This should switch off the timer of the shell
   b. TIMER=3
      Now, timer message should appear after 3 seconds, in case of step 1.(as above)
3. To run a process in a background (additional functionlity)
    > ./test &
	> ps ax 
		You can still see the "./test" running in process list.
    > kill -9 <PID of ./test>
		This should kill the process
4. To check if Alias command is working
	> Alias ls1='ls'
	> ls1
		This should work as "ls"
5.  Special case 
	> Alias ls='ls1'
	> Will not work as ls can't be used as Alias name. 
6. If - then - else -fi parsing
	> If ls; then pwd; else clear; 
		If ls command gets executed without any problem then pwd will be executed. 
		If ls fails to get executed then clear command will be executed.  
7. >exit command can be used to quit the prompt.  