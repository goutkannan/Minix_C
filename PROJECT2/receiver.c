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

	for(j=1;j<argc;j++)
	{
		r = atoi(argv[j]);

		i |= (1<<r);

			message m1;
			m1.m1_i1 = (int)i;
			m1.m1_p1 = p;

			err = retrieve(m1);
		
			if(err == -2)
				exit(1);
			else if(err == 1)
				printf("\n Received Message is %s \n",p);

			strcpy(p,"");
	 }

		return(0);

}
