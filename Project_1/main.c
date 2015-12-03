#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include  <signal.h>
#include  <sys/ipc.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define LSH_RL_BUFSIZE 1024

#define MAX_LENGTH 500
#define numalias 500
char **(alias_str);
char **(alias_value);
int alias_count=0;
int exec_flag=100;
int new_sh_ifthen(char**);
int Alias_implementation(char** );

pid_t pid1;


int run_exec(char **);

char *read_cmd_line()
{
  char *line = NULL;

  ssize_t bufsize = 0; // have getline allocate a buffer for us

  getline(&line, &bufsize, stdin);
  return line;
}

char **split_cmd_line(char *line)
{
  int buffsize = 50, pos = 0;
  char *token;
  char **tokenpos = malloc(buffsize * sizeof(char*));

  if (!tokenpos) {
    fprintf(stderr, "sh: no memory to allocate, re-run ur shell\n");
    exit(0);
  }

  token = strtok(line, " \a\r\n\t");
  while (token != NULL) {
    tokenpos[pos] = token;
    pos++;

    if (pos >= buffsize) {
      buffsize = buffsize + 50;
      tokenpos = realloc(tokenpos, buffsize * sizeof(char*));
      if (!tokenpos) {
        fprintf(stderr, "sh: no memory to allocate, re-run ur shell\n");
        exit(0);
      }
    }

    token = strtok(NULL, " \a\r\n\t");
  }
  tokenpos[pos] = NULL;
  return tokenpos;
}

void chld_SIGQUIT_handler(int num)
{
exit(0);
}
int new_sh_exit()
{
	printf("\n Thank You..Bye..\n");
  exit(0);
}
int loadalias()
{
	       FILE *fp;
	       char *line = NULL;
	       char *tok = NULL;
	       const char c=':';
	       size_t len = 32;
	       ssize_t read;
	       int i=0;
	       alias_value = malloc(numalias * sizeof(char*));
	       alias_str = malloc(numalias * sizeof(char*));
	       fp = fopen("Alias_list.txt", "r");
	       if (fp == NULL)
	           return(0);

	       while ((read = getline(&line, &len, fp)) != -1)
	       {
	          //printf("\n Line from File : %s",line);


	          tok = strtok(line,":");
	         if(tok!=NULL)
	         {
	        	 alias_str[i] = malloc((MAX_LENGTH+1) * sizeof(char));

				 alias_value[i] =  malloc((MAX_LENGTH+1) * sizeof(char));


	        	 strcpy(alias_str[i],tok);
	        	 //printf("\n%s-----",tok);
	        	 tok = strtok(NULL," \n\r\t");
	        	 if(tok!=NULL)
	        		 strcpy(alias_value[i],tok);
	        	 //printf("\n%s-----",alias_value[i]);
	        	 i++;
	         }




	       }
	       alias_count = i;
	       fclose(fp);
	       if (line)
	           free(line);
	       return (1);

}
int parse_args(char **args)
{

	int status=0;
	int i,c,j,flag=0,pos;
	char **new_args =  malloc(numalias * sizeof(char*));
	char **arg_token;

	if (alias_count==0)
		loadalias();

	if (strcmp(args[0],"exit")==0)
	{
		status = new_sh_exit();
	}
		else if (strcmp(args[0],"If")==0)
	{
		status = new_sh_ifthen(args);
		return status;
	}
		else if (strcmp(args[0],"Alias")==0)
	{
		status = Alias_implementation(args);
	}
		else
	{
		for (i = 0; i < alias_count; i++)  //Check if the command is in alias txt
			  {
				 // printf("\n In for loop  %s comparing %s \n",args[0],alias_str[i]);
				 if (strcmp(args[0], alias_str[i]) == 0)
				 {
					 pos=i;
					 flag =1;
					// printf("\nMatch for %s is %s at %d",alias_str[i] , alias_value[i],i);
				 }
			  }

			  if (flag==1)
			  {

				  arg_token = split_cmd_line(alias_value[pos]);
				 // printf("\n first--%s and size is %d",arg_token[0],sizeof(arg_token)/sizeof(arg_token[0]));

				 		for(j=0;j<sizeof(arg_token)/sizeof(arg_token[0]);j++) //get the alias line tokenized and copy to the new argument parameter
				 		{
				 			new_args[j] = malloc((MAX_LENGTH+1) * sizeof(char));
				 			stpcpy(new_args[j],arg_token[j]);

				 		}
				 		//printf("\n inside for at %d ----> %s",sizeof(args)/sizeof(args[0]),new_args[j-1]);

				 		if(sizeof(args)/sizeof(args[0])>1)
				 		{
				 			for(c=1;c<sizeof(args)/sizeof(args[0]);c++)  // copy the other params given by the user onto the args array
				 			{
				 				new_args[j] = malloc((MAX_LENGTH+1) * sizeof(char));
				 				stpcpy(new_args[j++],args[c]);
				 			}
				 		}
				 		parse_args(new_args);

			  }
			  else if (flag==0)
			  {
				  //printf("Arg sent is %s",args[0]);
				  return run_exec(args);
			  }
		 }

	return status;

}

int lsh_launch(char **argv,char **pass,char **fail)
{
	pid_t  pid;
	     int    status;

	     if ((pid = fork()) < 0) {     /* fork a child process           */
	          printf("*** ERROR: forking child process failed\n");

	          exit(1);
	     }
	     else if (pid == 0) {          /* for the child process:         */
	          if (execvp(*argv, argv) < 0) {     /* execute the command  */
	               printf("*** ERROR: exec failed\n");

	               exit(errno);
	          }
	     }
	     else {                                  /* for the parent:      */
	          while (wait(&status) != pid)       /* wait for completion  */
	               ;
	          if(WIFEXITED(status))
	          {
	                   if ((WEXITSTATUS(status))==0)
	                   {
	                	   //printf("\n Passed condition ");
	                	   run_exec(pass);
	                   }
	                   else
	                   {
	                	   //printf("\n Failed condition");
	                	   run_exec(fail);
	                   }
	          }
	     }
}

int check_not_exists(char* key)
{
	int count=0;
	//printf("\n Checking for %s",key);
	for(count=0;count<alias_count;count++)
	{
		if ((strcmp(alias_str[count],key)==0) || (strcmp(alias_value[count],key)==0))
		{
			printf("\n Alias can't be set, Please use a different Alias \n",alias_str[count],alias_value[count]);
			return 0;
		}

	}
	return 1;

}
int appendalias(char* key,char* value)
{
	//printf("\n In Append Alias %d",alias_count);
	alias_count++;

	alias_str[alias_count] = malloc((MAX_LENGTH) * sizeof(char));
	alias_value[alias_count] =  malloc((MAX_LENGTH) * sizeof(char));

//	printf("\nKey - %s- value-%s",key,value);
	strcpy(alias_str[alias_count],key);
	strcpy(alias_value[alias_count],value);
	return 1;
}
int addtofile(char** data,int counter)
{
	FILE *fp;
	char *k,*v;
	int j,i=0,c=0;

	//printf("\n%s-----------%s------%d\n",data[0],data[1],counter);

	fp = fopen("Alias_list.txt","ab+");
	while(i<counter)
	{

		//printf("\nlength %d",strlen(data[i+1]));
	    if (strlen(data[i])>0 && strlen(data[i+1])>0)
	    {
	    	k = data[i];
			v = strtok(data[i+1],"'");
			int s = fprintf(fp,"%s:%s \n",data[i],v);
			i=i+2;
	    }
	    else
	    	 printf("Alias Syntax Error\n");
	}
	fflush(fp);
	fclose(fp);

	return (appendalias(k,v));
}
int Alias_implementation(char** args)
{
	char* tokens;
		char **line=malloc(numalias * sizeof(char*));;
		int count=0;
		if(!(strcmp(args[0],"Alias")))
		{

			tokens = strtok(args[1],"=");
			//printf("\n alais_implementation Args--%s-%s--tokens-- %s\n",args[0],args[1],tokens);
			while(tokens!=NULL)
			{
				line[count]=malloc((MAX_LENGTH+1) * sizeof(char));

				line[count]=tokens;
				//printf("\n %s\t%d",line[count],count);
				count++;
				tokens=strtok(NULL," \n\r\t");
			}
			if(count==0)
			{
			 printf("Need to print all the Alias in the list");

			}
			else if(count<2)
			{
						printf("Check Alias Syntax");
			}
			else if(count>1)
			{
				if(check_not_exists(line[0]))
					addtofile(line,count-1);

			}

			 return 1;
		}
		else
		{
			//printf("wrong");
			return 0;
		}

}
int new_sh_ifthen(char** args)
{
	int i=1;
	char **condition,**passaction,**failaction;
	condition =  malloc(LSH_TOK_BUFSIZE * sizeof(char*));
	passaction = malloc(LSH_TOK_BUFSIZE * sizeof(char*));
	failaction = malloc(LSH_TOK_BUFSIZE * sizeof(char*));
int flag=0;

	 if(args[i]!=NULL)
	 {
		     int j=0;
		 	 while(1)
		 	 {

		 		if(args[i]==NULL)
		 		{

		 			exit(0);
		 		}

		 		else if(args[i][strlen(args[i])-1]==';')
		 		{

		 			condition[j] = malloc((MAX_LENGTH+1) * sizeof(char));
		 		    strncpy(condition[j],args[i],strlen(args[i])-1);

		 			i++;
		 			j++;
		 			break;
		 		}
		 		else
		 		{
		 			condition[j] = malloc((MAX_LENGTH+1) * sizeof(char));

		 			strcpy(condition[j],args[i]);
		 			i++;
		 			j++;
		 		}
		 	 }

		 	 if(strcmp(args[i],"then")==0)
		 	 {
		 			j=0;
		 			i++;
					while(1)
					{
						if(args[i]==NULL)
						{

							exit(0);
						}
						else if(args[i][strlen(args[i])-1]==';')
						{

							passaction[j] = malloc((MAX_LENGTH+1) * sizeof(char));
							strncpy(passaction[j],args[i],strlen(args[i])-1);

							i++;
							j++;
							break;
						}
						else
						{
							passaction[j] = malloc((MAX_LENGTH+1) * sizeof(char));
							strcpy(passaction[j],args[i]);
							i++;
							j++;
						}
					}

					if(strcmp(args[i],"else")==0)
					{
						i++;
						while(1)
						{   j=0;
							if(args[i]==NULL)
							{
								printf("\n Else action syntax is wrong");
								exit(0);
							}
							else if(args[i][strlen(args[i])-1]==';')
							{
								//printf("\n In while-else-';' - %s",args[i]);
								failaction[j] = malloc((MAX_LENGTH+1) * sizeof(char));
								strncpy(failaction[j],args[i],strlen(args[i])-1);
								//printf("\n In while-else condition - %s",failaction[j]);
								i++;
								j++;
								break;
							}
							else
							{
								failaction[j] = malloc((MAX_LENGTH+1) * sizeof(char));
								strcpy(failaction[j],args[i]);
								i++;
								j++;
							}
						}
						if(args[i]!=NULL)
						{
							if(strcmp(args[i],"fi")==0)
							{
								flag=1;
							}
						}
					}
					else
					{
							 printf("\nMissing Else Keyword\n");
							 return(0);
					}
		 	 	 }
		 		 else
		 		 {
		 			 printf("\n 'then' Keyword missing after If Condition\n");
		 			return(0);
		 		 }

	}
	 else
		 {
		 	 printf("\nCondition is not present in If \n");
		 	return(0);
		 }
if(flag==1)
{

		printf("\n Executing condition %s",condition[0]);

		lsh_launch(condition,passaction,failaction);

return 1;

}
else
{
	printf("\n If ...; then ...; else ...; fi is the expected syntax \n");
	return 0;
}
}

int run_exec(char **args)
{

  int status;
  clock_t start, end, total;
  char c;
  pid_t pidp, pid2, wpid;
  pidp = getpid();
  pid1 = fork();
  if (pid1 == 0) {
    if (execvp(args[0], args) == -1) {
      perror("sh: couldnot exec, try re-run");
    }
    exit(0);
  } else if (pid1 < 0) {

    perror("sh: couldnot fork, try re-run");
  } else {
  pid2 = fork();
  if (pid2 == 0) {
 if (signal(SIGQUIT, chld_SIGQUIT_handler) == SIG_ERR) {
          printf("SIGINT install error\n");
          exit(1);
     }

  while(1){
  sleep(5);
	printf("Do you want to terminate the current command[Y/N]:");
c=getchar();
getchar();
fflush(stdin);
if (c == 'Y' || c == 'y') {
kill(pidp, SIGCHLD);
exit(0);
}
}
	}
	do{
      wpid = waitpid(pid1, &status, WUNTRACED);
	
	}while(!WIFEXITED(status) && !WIFSIGNALED(status));

}
kill(pid2, SIGQUIT);
	do{
      wpid = waitpid(pid2, &status, WUNTRACED);
	
	}while(!WIFEXITED(status) && !WIFSIGNALED(status));

return 1;
 
}
void SIGCHLD_handler(int num)
{
kill(pid1, SIGQUIT);
}

int main( void ) {
char *line;
  char **args;
  int status;
 if (signal(SIGCHLD, SIGCHLD_handler) == SIG_ERR) {
          printf("SIGINT install error\n");
          exit(1);
     }
  
 while(1)
 {
printf(">");
 line = read_cmd_line();
      args = split_cmd_line(line);

    status = parse_args(args);
}
	/*	int pid = fork();
	if ( pid == 0 ) {
  	execvp(argv[0], argv);
	}
        else {
	wait(&status);
}
*/
free(alias_str);
free(alias_value);
	return 0;
}
