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
#include <signal.h>



void extract(char** input, char**output, int* i_len, int* o_len, command_t cmd){
	switch (cmd->type)
    {
	    case AND_COMMAND:
	    case SEQUENCE_COMMAND:
	    case OR_COMMAND:
	    case PIPE_COMMAND:
	    {
	    	extract(input, output, i_len, o_len, cmd->u.command[0]);
	    	extract(input, output, i_len, o_len, cmd->u.command[1]);
	    	break;
	    }
	    case SIMPLE_COMMAND:
	    {
	    	char* term = strtok(*(cmd->u.word), " ");
	    	bool wasExec = false;
	    	while(term!=NULL) {
		        if(0 == strcmp(term, "exec")){ 
		        	wasExec = true;
		        }
		        else if(wasExec){
		        	wasExec=false;
		        }
		        else{
		          if((*i_len+1)%20==0) {
		          	input = (char**) realloc(input, sizeof(char*)*2*(*i_len+1));
		          }
		          input[*i_len] = (char*)malloc(strlen(term));
		          memcpy(input[*i_len], term, strlen(term));
		          *i_len++;
		        }
		        term = strtok(NULL, " ");
		    }
		    //add commands input and output
		    if((*i_len+1)%20==0) {
	          	input = (char**) realloc(input, sizeof(char*)*2*(*i_len+1));
	        }
	        
	        input[*i_len] = (char*)malloc(strlen(cmd->input));
	        memcpy(input[*i_len], cmd->input, strlen(cmd->input));
	        *i_len++;
    	
	        if((*o_len+1)%20==0) {
	          	output = (char**) realloc(output, sizeof(char*)*2*(*o_len+1));
	        }
            output[*o_len] = (char*)malloc(strlen(cmd->output));
            memcpy(output[*o_len], cmd->output, strlen(cmd->output));
            *o_len++;
	    	break;
	    }
	    case SUBSHELL_COMMAND: 
	    {      
	     	 //add commands input and output
		    if((*i_len+1)%20==0) {
	          	input = (char**) realloc(input, sizeof(char*)*2*(*i_len+1));
	        }
	        
	        input[*i_len] = (char*)malloc(strlen(cmd->input));
	        memcpy(input[*i_len], cmd->input, strlen(cmd->input));
	        *i_len++;
    
	        if((*o_len+1)%20==0) {
	          	output = (char**) realloc(output, sizeof(char*)*2*(*o_len+1));
	        }
            output[*o_len] = (char*)malloc(strlen(cmd->output));
            memcpy(output[*o_len], cmd->output, strlen(cmd->output));
            *o_len++;

            //extract from subshell_command
	        extract(input, output, i_len, o_len, cmd->u.subshell_command);
	    	break;
	    }
	    default:
        	break;
    }
}

struct command_io* create_command_io(command_t cmd) {
	struct command_io* new_c = (struct command_io*)malloc(sizeof(struct command_io));
	new_c->c = cmd;
	new_c->pid = -1;
	new_c->i_len = 0;
	new_c->o_len = 0;
	new_c->isRunning = false;
	extract(new_c->inputs, new_c->outputs, &(new_c->i_len), &(new_c->o_len), cmd);

	return new_c;
}

bool isDependent(command_t first, command_t second){
	bool result = false;

	return result;
}

void handle_process(int sig) {
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset, SIGINT);
	sigprocmask(SIG_BLOCK, &sset, NULL);
	printf("handling signal\n");
	sleep(2);
	printf("handling signal again\n");
	sigprocmask(SIG_UNBLOCK, &sset, NULL);
	//_exit(1);
}

void create_p(){
	int grand_p = fork();
	if(grand_p==0){
		//printf("child doing stuff\n");
		//sleep(2);
		raise(SIGINT);
		_exit(1);
	}
	else{
		//printf("main stuff\n");
	}
}


int
main (int argc, char **argv)
{
	/*
	signal(SIGINT, handle_process);
	//signal(SIGUSR1, handle_process);
	int i;
	for(i=0; i<3; i++){
		create_p();
		//sleep(1);
	}
	*/
}