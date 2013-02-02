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
char** DEP;
int NUM_O_COMMANDS;
struct command_io* CMD_SPOT;
pid_t* CHILDREN;
static int FAIL = 123;
static int execute_normally(command_t c);
int command_status (command_t c)
{
  return c->status;
}

static int execute_time_travel(command_t c);
int execute(char* command, char* input, char* output);
 
int execute_command (command_t c, bool time_travel) {
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  if(time_travel){
    //do time travel stuff
    return execute_time_travel(c);
  }
  else{
     return execute_normally(c); 
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

    return execute(*c->u.word, c->input, c->output);
    }

    case SUBSHELL_COMMAND: 
    {      
      pid_t sp = fork();
      if(sp == 0){
        int fp = -1;
        if(c->input != NULL) {
          int fd = open(c->input, O_RDONLY);
          if(fd < 0) {
              error(0, 0, "Failed to open file: %s\n", c->input);
              _exit(FAIL);
          }
          dup2(fd, 0);
        }
        if(c->output != NULL) { 
          fp = open(c->output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
          if(fp < 0) {
              error(0, 0, "Failed to open file: %s\n", c->output);
              _exit(FAIL);
          }
          dup2(fp, 1);
        }

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
    int fp = -1, fd = -1;
    if(input != NULL) {
      fd = open(input, O_RDONLY);
      if(fd < 0) {
          close(fd);
          error(0, 0, "Failed to open file: %s\n", input);
          return 0;
      }
      dup2(fd, 0);
    }
    if(output != NULL) { 
      fp = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if(fp < 0) {
          close(fp);
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
      if(strncmp(arr[0], "false", 5)==0)
        exit(FAIL);

      execvp(arr[0], arr);
      error(FAIL, 0, "Command failed: %s", command);
      exit(FAIL);
    }
    else{
      if(input != NULL && fd>0) {
        dup2(orig, 1);
        close(fd);
      }
      if(output != NULL && fp>0) { 
        dup2(orig, 1);
        close(fp);
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

static int execute_time_travel(command_t c){
 if(c){}

return 1;


}
void init(int len){
	DEP = (char**)malloc(len*sizeof(char*));
	NUM_O_COMMANDS = len;
	int i;
	for(i=0; i < len; i++){
		DEP[i] = (char*)malloc(len*sizeof(char));
	}
        //initialize to X
        int j;
	for(i=0; i < len; i++){
	   for(j=0; j < len; j++){
		DEP[i][j] = 'x';
	   }
	}
	CMD_SPOT = (struct command_io*)malloc(len*sizeof(struct command_io));
	for(i = 0; i < len; i++){
		CMD_SPOT[i].pid = -1;//i's pid DNE yet
	}	
	//What about CHILDREN?
}

void remove_globs(){
	int i;
	for(i = 0; i < NUM_O_COMMANDS; i++){
		free(DEP[i]);
	}
	free(DEP);
	free(CMD_SPOT);
}

void add_dep(int cmd_num){
//add command number and fill out dependencies in matrix DEP
	if(cmd_num >= NUM_O_COMMANDS) return;
	DEP[cmd_num][cmd_num] = 'f';
	//check dependencies and fill out matrix
	//check i only if DEP[i][i] is 'f'
	int i;
	for(i = 0; i < cmd_num; i++){
		if(DEP[i][i] == 'f'){//it is waiting or running
			//check input outputs
			//check if this input is that output
			//check if this ouput is that input
			//we are interested in CMD_SPOT[i].input and .output
			//which are arrays of strings
			
		}
	}
	run_non_dep();
}

void remove_dep(int cmd_num){
//remove command from matrix DEP 
//clear column and row by chaning to X
	int i;
	for(i = 0; i < NUM_O_COMMANDS; i++){
		DEP[i][cmd_num] ='f';
	}
//run any cleared procs
	run_non_dep();
}

void run_non_dep(){
//check for procs which can run, and run them.
	int i,j;
	for(i = 0; i < NUM_O_COMMANDS; i++){
		for(j = 0; j <= i; j++){
			if(DEP[i][j] != 'f')
				break;
			if(j == i)
				{;}
				//run CMD_SPOT[i].c
		}
	}
}
