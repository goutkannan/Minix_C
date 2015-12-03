#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<lib.h>
#include"mailbox.h"
int main(int argc, char *argv[])
{
	char p[30]="";
	int j=0,r,err;
	unsigned int i= 0 ;
	//printf("\n args %d",argc);
	for(j=1;j<argc;j++)
	{
		r = atoi(argv[j]);
		//printf("\n %u",r);
		i |= (1<<r);
		int c;
		for(c=0;c<101;c++)
		{
			printf("\n %d",c);
			message m1;
			m1.m1_i1 = (int)i;
			m1.m1_p1 = p;

			err = retrieve(m1);
			printf("\n Err no %d",err);
			if(err == -2)
				exit(1);
			else if(err == 1)
				printf("\n Received Message is %s \n",p);
			else if (err==-1)
				printf("\n No message is available for you\n");
			strcpy(p,"");
		}
	}
	return(0);
	
}

