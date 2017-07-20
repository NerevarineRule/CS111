// UCLA CS 111 Lab 1 command execution
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "command-internals.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
void enque(struct queue** qin, struct command* cin)
{
	if (*qin == NULL) //if empty, add an element to the top of the queue;
	{
		struct queue* temp = (struct queue*)malloc(sizeof(struct queue));
		temp->ptr = cin;
		temp->next = *qin;
		*qin = temp;
	}

	else //otherwise, put the element to the back of the stack
	{
		struct queue* temp = (struct queue*)malloc(sizeof(struct queue));
		temp->ptr = cin;
		temp->next = NULL; 
		struct queue* head = *qin;
		while (head != NULL)
		{
			if (head->next == NULL)
			{
				head->next = temp;
				return;
			}
			head = head->next;
		}
		
	
	}
}

void deque(struct queue** qin) //remove the topmost element
{
	if (*qin != NULL)
	{
		struct queue* temp = *qin;
		*qin = temp->next;
		free(temp);		
	}

}



int
command_status (command_t c)
{
  return c->status;
}

pid_t execute_simple_command(command_t command)
{
		pid_t pid;
		int status;
		//int fdo;
		//int fdi;
		int in = dup(0);
		int out = dup(1);

		if( !strcmp(command->u.word[0], "exec") ) 
		{
			execvp(command->u.word[1], command->u.word+1);
			exit(1); //command not found = 127 
		}
/*
		if(command->output)
		{
			if((fdo = open(command->output, O_WRONLY, O_TRUNC)) < 0)
			{
				fprintf(stderr, "Error, write failed\n");
				exit(1);
			}
			dup2(fdo, 1);
		}
		if(command->input)
		{
			if((fdi = open(command->input, O_RDONLY)) < 0)
			{
				fprintf(stderr, "Error, read failed\n");
				exit(1);
			}
			dup2(fdi, 0);
		}*/
		if((pid = fork()) < 0)
		{
			fprintf(stderr, "Error, fork failed\n");
			exit(1);
		}
		else if(pid == 0)
		{
			
			if(command->output)
			{
				int fdo;
				if((fdo = creat(command->output,0644)) < 0)
				{
					fprintf(stderr, "Error, write failed\n");
					exit(1);
				}
				dup2(fdo, 1);
				close(fdo);
			}
			if(command->input)
			{
				int fdi;
				if((fdi = open(command->input, O_RDONLY)) < 0)
				{
					fprintf(stderr, "Error, read failed\n");
					exit(1);
				}
				dup2(fdi,0);
				close(fdi);
			}

			if(execvp(command->u.word[0], command->u.word) < 0)
			{
				fprintf(stderr, "Error, exec failed\n");
				exit(1);
				
			}
		}
		else
		{
			waitpid(pid,&status,0);
			dup2(in,0);
			dup2(out,1);
			close(in);
			close(out);
			command->status = WEXITSTATUS(status);
		}
		return pid;
}

pid_t execute_subshell_command(command_t command)
{
		if(command->output)
		{
			int fdo;
			if((fdo = creat(command->output,0644)) < 0)
			{
				fprintf(stderr, "Error, write failed\n");
				exit(1);
			}
			dup2(fdo, 1);
			close(fdo);
		}
		if(command->input)
		{
			int fdi;
			if((fdi = open(command->input, O_RDONLY)) < 0)
			{
				fprintf(stderr, "Error, read failed\n");
				exit(1);
			}
			dup2(fdi, 0);
			close(fdi);
		}
		
		pid_t pid = execute_command(command->u.subshell_command, false);
		command->status = command->u.subshell_command->status;		
		return pid;
}

pid_t execute_sequence_command(command_t command)
{
		
		pid_t pid = execute_command(command->u.command[0], false);
		command->status = command->u.command[0]->status;	
		pid = execute_command(command->u.command[1], false);
		command->status = command->u.command[1]->status;	
		return pid;
}

pid_t execute_and_command(command_t command)
{
		
		pid_t pid = execute_command(command->u.command[0], false);
		command->status = command->u.command[0]->status;
		if(command->u.command[0]->status == 0)
		{
			pid = execute_command(command->u.command[1], false);
			command->status = command->u.command[1]->status;	
		}
		return pid;
}

pid_t execute_or_command(command_t command)
{
		
		pid_t pid = execute_command(command->u.command[0], false);
		command->status = command->u.command[0]->status;
		if(command->u.command[0]->status != 0)
		{
			pid = execute_command(command->u.command[1], false);
			command->status = command->u.command[1]->status;	
		}
		return pid;
}

pid_t execute_pipe_command(command_t command)
{
		int status;
		int status2;
		pid_t pid;
		int fd[2];
		pid_t pipepid;
		if((pipepid = fork()) < 0)
		{
			fprintf(stderr, "Error, fork failed\n");
			exit(1);
		}
		else if(pipepid == 0)
		{
			if((pipe(fd)) < 0)
			{
				fprintf(stderr, "Error, pipe failed\n");
				exit(1);
			}
			if((pid = fork()) < 0)
			{
				fprintf(stderr, "Error, fork failed\n");
				exit(1);
			}
			else if(pid == 0)
			{
				dup2(fd[1], 1);
				close(fd[0]);
				pid = execute_command(command->u.command[0], false);
				close(fd[1]);
				exit(command->u.command[0]->status);
			}
			else
			{
				waitpid(pid, &status2, 0);
				dup2(fd[0], 0);
				close(fd[1]);
				pid = execute_command(command->u.command[1], false);
				close(fd[0]);
			}
			exit(command->u.command[1]->status);
		}
		else
		{
			waitpid(pipepid, &status, 0);
			command->status = WEXITSTATUS(status);
			return pipepid;
		}
}



pid_t execute_command (command_t c, bool time_travel)
{
	pid_t pid;
	if(!time_travel)
	{
		switch(c->type) 
		{
			case SIMPLE_COMMAND:
			{
				pid = execute_simple_command(c);
				break;		
			}	

			case SUBSHELL_COMMAND:
			{
				pid = execute_subshell_command(c);
				break;
			}

			case SEQUENCE_COMMAND:
			{
				pid = execute_sequence_command(c);
				break;
			}
		
			case PIPE_COMMAND:
			{
				execute_pipe_command(c);
				break;
			}
	
			case AND_COMMAND:
			{
				execute_and_command(c);
				break;
			}

			case OR_COMMAND:
			{
				execute_or_command(c);
				break;
			}
			default:
			{}
		}
	}
	return pid;
}


void execute_command_t(command_stream_t cstream,command_t* last_command)
{

  struct queue* independent = NULL;
  struct queue* dependent = NULL;	
  command_t command;
  int icoms = 0;
  int dcoms = 0;
  //build dependency graphs
  
  while ((command = read_command_stream (cstream)))
  {
	if (command->bsize == 0)
	{
		enque(&independent,command);
		icoms++;
	}	
	
	else
	{
		enque(&dependent,command);
		dcoms++;
	}
  }

  //while a queue of pointers to commands that have no dependency are not empty
  while (independent != NULL)
  {
	int status;
	pid_t pid = fork();
	if (pid == 0)
	{	
		execute_command(independent->ptr,false);
		status = independent->ptr->status;
		exit(status);
	}
	
	else
	{	
		*last_command = independent->ptr;
		independent->ptr->pid = pid; 
	}
	deque(&independent);

  } 
 
  int status;
  while (icoms > 0) 
  {
	wait(&status);
	icoms--;
  } 
  //while a queue of pointers to commands that have dependency are not empty
  while (dependent != NULL)
  {
	int status;
	int i,limit;
	int requeue = 0;
	command_t toback;
	limit = dependent->ptr->bsize;
	for (i = 0; i < limit; i++)
	{
		if (dependent->ptr->before[i]->pid == 0) //if pid is undefined, put the command back to queue
		{
			requeue = 1;
			toback = dependent->ptr;
			deque(&dependent);
			enque(&dependent, toback);
			break; //return to the while loop.
		}
	}
	//above loop handles the case when we accidently pop a command that is still dependent
	if(requeue == 0)
	{
		for (i = 0; i < limit; i++) //make sure all the processes that depend on current command finish before executing current command.
		{ 
			waitpid(dependent->ptr->before[i]->pid,&status,0);
		}
		pid_t pid = fork();
		if (pid == 0) //child
		{	
			execute_command(dependent->ptr,false);
			exit(dependent->ptr->status);
		
		}

		else //parent
		{
			*last_command = dependent->ptr;
			dependent->ptr->pid = pid;
		}
		deque(&dependent);
	}
  }
  while (dcoms > 0) 
  {
	wait(&status);
	dcoms--;
  }

  //last_command = NULL;
  return;

}

 
