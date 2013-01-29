// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

static int FAIL = 123;
static int execute_normally(command_t c);
int command_status (command_t c)
{
  return c->status;
}

int execute(char* command, char* input, char* output);
 
void execute_command (command_t c, bool time_travel) {
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

	if(time_travel){
		//do time travel stuff
	}
	else{
	   execute_normally(c);	
	}

}
static int execute_normally(command_t c){
  switch (c->type)
  {
    case AND_COMMAND:
    {
	     if(execute_normally(c->u.command[0])) {
		      return execute_normally(c->u.command[1]);
	     }
	     else //left side failed
		      return 0;
    }
    case SEQUENCE_COMMAND:
    {
	     int temp_ = 0;
	     if(!execute_normally(c->u.command[0])) { temp_++; }
	     if(!execute_normally(c->u.command[1])) { temp_++; }
	     return !temp_;
    }
    case OR_COMMAND:
    {
	     if(execute_normally(c->u.command[0])) {
		      return 1;
	     }
	     else //left side failed
		      return execute_normally(c->u.command[1]);
    }
    case PIPE_COMMAND:
    {
        pid_t p1 =fork();
        if(p1==0) {//child  
          int fd[2];

          pipe(fd);
          //printf("%s | %s\n", left, right);
          pid_t p = fork();
          if(p==0) { //child
              close(fd[0]); //close fd thats not being used 
              dup2(fd[1], 1); //duplicate fd1 to stdout
              close(fd[1]);
              if(!execute_normally(c->u.command[0]))
                exit(FAIL);
          }
          else { //parent process
              close(fd[1]);
              dup2(fd[0], 0); //duplicate fd0 to stdin
              close(fd[0]);

              //check for error
              
              int status;
              if(waitpid(p, &status, 0)<0) 
                  return 0;
            
              if(WEXITSTATUS(status) == FAIL ) {
                  kill(p, SIGKILL);
                  return 0;    
              }

              //printf("%s\n", right);
              //execute command
              char* term = strtok(*(c->u.command[1]->u.word), " ");
              char* file = term;
              char* arr[100]; 
              int i = 0;
              while(term!=NULL) {
                  arr[i] = term;
                  term = strtok(NULL, " ");
                  i++;
              }
              arr[i]=NULL;

              //sleep(1);
              execvp(file, arr);
              exit(FAIL);
          }
        }
        else { //parent
              //check for error
              int status;
              if(waitpid(p1, &status, 0)<0) 
                  return 0;
            
              if(WEXITSTATUS(status) == FAIL ) {
                  kill(p1, SIGKILL);
                  return 0;    
              }
              kill(p1, SIGKILL);
              return 1;
        }
	
    }
    case SIMPLE_COMMAND:
    {
	     //execute c->u.word which is a char **
	     //printf("Will execute this: %s\n",*c->u.word);
	     return execute(*c->u.word, c->input, c->output);
    }
    case SUBSHELL_COMMAND: 
    {      
	     return execute_normally(c->u.subshell_command);
    }
    default:
        abort ();
    }

//  if (c->input)
//    printf ("<%s", c->input);
//  if (c->output)
//    printf (">%s", c->output);

	 return 1;
}



int execute(char* command, char* input, char* output) {
    if(command == NULL) { return 0; }
	  char* term = strtok(command, " ");
 
    int orig = dup(1);
    int fp = -1;
    if(input != NULL) {
      int fd = open(input, O_RDONLY);
      if(fd < 0) {
        //print error saying file doesnt exist
      }
      dup2(fd, 0);
    }
    if(output != NULL) { 
 	    fp = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0666);

	    dup2(fp, 1);
    }
    pid_t p = fork();
    if(p==0){ //child
    	char* file = term;
    	char* arr[100]; 
    	int i = 0;
    	while(term!=NULL) {
    		arr[i] = term;
    		term = strtok(NULL, " ");
    		i++;
    	}
    	arr[i]=NULL;

    	execvp(file, arr);
		  _exit(FAIL);
    }
    else{
      if(fp != -1){
    	   close(fp);
    	   dup2(orig, 1);
	    }
    	int status;
    	if(waitpid(p, &status, 0)<0) {
    		  return 0;
    	}
	    if(WEXITSTATUS(status) == FAIL ) {
    		  kill(p, SIGKILL);
		      return 0;    
	     }
	    kill(p,SIGKILL);	

	    return 1;
    }

	return 0;
}


