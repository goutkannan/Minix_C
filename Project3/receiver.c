#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<lib.h>
#include"mailbox.h"
int main(int argc, char *argv[])
{
	char p[30]="";
	int j=0,r,err,gid;
	unsigned int i= 0 ;
	if (argc<=2)
	{
		printf("\nError: No receiver Found ");
		return 0;
	}
	if(argc<=1)
	{
		printf("\n Group Id not found");
		return 0;
	}

	gid = atoi(argv[1]);
	for(j=2;j<argc;j++)
	{
		r = atoi(argv[j]);
		i |= (1<<r);
		err = retrieve(gid,i,p);
		if(err == -1)
		{
			printf("No Messages for you \n");
			exit(1);
		}
		else if(err == 1)
			printf("\n Received Message is %s \n",p);
		else if(err==0)
			printf("You have reached an empty Mailbox  \n");
		strcpy(p,"");
	 }

		return(0);

}
