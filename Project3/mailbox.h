#include<stdlib.h>
#include<lib.h>

message m1;

#define clear()	_syscall(PM_PROC_NR,PM_CLEAR, &m1)

int retrieve(int gid,int i, char* p)
{
	//*message m1;
	if(i<=1)
	{
		printf("\n Invalid Receiver Name: Hint Receiver must be greater that 0 and it must be an integer");
		return 0;
	}
	m1.m1_i1 = (int)gid;
	m1.m1_i2 = (int)i;
	m1.m1_p1 = p;
	int temp;
	temp =  _syscall(PM_PROC_NR,PM_RETRIEVE, &m1);

	return temp;
}
int deposit(int gid,int r, char* p)
{
	if(r<=1)
	{
			printf("\n Invalid Receiver Name: Hint Receiver must be greater that 0 and it must be an integer");
			return 0;
	}

	m1.m1_i1 = (int)gid;
	m1.m1_i2 = (int)r;
	m1.m1_p1 = p;

	return _syscall(PM_PROC_NR,PM_DEPOSIT, &m1);
}
#define setup(m1)		_syscall(PM_PROC_NR,PM_SETUP, &m1)
