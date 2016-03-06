#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<lib.h>
#include"mailbox.h"

int main(int argc, char *argv[])
{
	char p[30];
	unsigned int r=0 ;
	int i=0,j;

int check=0;
strcpy(p,argv[1]);
	//while(check < 2)
	//{

		//strcat(p,"A");
		//check++;
		for(j=2;j<argc;j++)
		{
			i=atoi(argv[j]);
			message m1;
			r |= (1<<i);

			m1.m1_i1 = (int)r;
			m1.m1_p1 = p;
			deposit(m1);
		}

 //}
	return(0);
}

