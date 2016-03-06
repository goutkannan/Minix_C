#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<lib.h>
#include"mailbox.h"

int main(int argc, char *argv[])
{
	char p[30];
	unsigned int r=0 ;

	int i=0,j,gid;

	int check=0;

	if (argc<4)
	{
		printf("Insufficient Inputs: **Message** **Receiver(atleast one)** should be present\n ");
		return 0;
	}
	strcpy(p,argv[1]);
	gid = atoi(argv[2]);

	//while(check < 2)
	//{

		//strcat(p,"A");
		//check++;

		for(j=3;j<argc;j++)
		{
			r=0;
			i=atoi(argv[j]);
			//		message m1;

			r |= (1<<i);

			deposit(gid,r,p);
		}

 //}
	return(0);
}

