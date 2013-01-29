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
#include <error.h>

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

          if(pipe(fd)<0)
            error(FAIL, 0, "Failed to create pipe.");
          //printf("%s | %s\n", left, right);
          pid_t p = fork();
          if(p==0) { //child
              close(fd[0]); //close fd thats not being used 
              dup2(fd[1], 1); //duplicate fd1 to stdout
              close(fd[1]);
              if(!execute_normally(c->u.command[0])) {
                exit(FAIL);
              }
              exit(1);
          }
          else { //parent process
              close(fd[1]);
              dup2(fd[0], 0); //duplicate fd0 to stdin
              close(fd[0]);

              //check for error
              
              int status;
              if(waitpid(p, &status, 0)<0) { 
                  exit(FAIL);
              }
            
              if(WEXITSTATUS(status) == FAIL ) {
                  kill(p, SIGKILL);
                  exit(FAIL);
              }
              kill(p, SIGKILL);
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
              error(FAIL, 0, "Failed on pipe command: |%s", *(c->u.command[1]->u.word) );
              exit(FAIL);
          }
          exit(1);
        }
        else { //grandparent
              //check for error
              int status;
              if(waitpid(p1, &status, 0)<0) {
                  return 0;
              }
            
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
	     //printf("Will execute this: %s\n",*c->u.word);i

		return execute(*c->u.word, c->input, c->output);
    }

    case SUBSHELL_COMMAND: 
    {      
	    pid_t sp = fork();
	    if(sp == 0){
		
  		if(0 == execute_normally(c->u.subshell_command)){
  			_exit(FAIL);
  		}
  		else {
  			_exit(987);
  	     }
  		}
	     else{
		int status;
		if(waitpid(sp, &status, 0)<0)
			return 0;
		if(WEXITSTATUS(status) == FAIL){
			kill(sp, SIGKILL);
			return 0;
		}
		kill(sp, SIGKILL);
		return 1;

        	}
   	 
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
          error(0, 0, "Failed to open file: %s\n", input);
  	      return 0;
  	  }
      dup2(fd, 0);
    }
    if(output != NULL) { 
 	    fp = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if(fp < 0) {
          error(0, 0, "Failed to open file: %s\n", output);
          return 0;
      }
	    dup2(fp, 1);
    }
    pid_t p = 0;
    char temp[5];
    strncpy(temp, command,4);
    temp[4] = '\0';
    //check if it is an exec command
    if(0 != strcmp("exec",temp)){ 
	     p = fork();
    }

    if(p==0){ //child
    	char* arr[100]; 
    	int i = 0;
    	while(term!=NULL) {
		    if(i == 0 && 0 == strcmp(term, "exec")){ 
        }
		    else{
    			arr[i] = term;
			    i++;
		    }
		    term = strtok(NULL, " ");
    	}
    	arr[i]=NULL;

    	execvp(arr[0], arr);
	    error(FAIL, 0, "Command failed: %s", command);
	    exit(FAIL);
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


