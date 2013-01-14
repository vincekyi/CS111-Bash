// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
struct cmd_n{
	command_t cmd;
	struct cmd_n* next;
};
typedef struct cmd_n cmd_node;

struct command_stream {
	cmd_node* commands;
	cmd_node* iterator;
};

void add_command(const char* command, command_t source){
	int semi_index = -1, andor_index = -1, pipe_index = -1;
	bool next_ch_pipe = false;
	bool next_ch_ampe = false;
	bool par_found = false;
	bool isLast = true;
	int cmd_len = strlen(command);
	char ch;
	int iter = cmd_len - 1;
	//printf("this is the command we are on: %s\n", command);
	while(iter > -1){
		ch = command[iter];
		switch(ch){ //how to deal with redirects???
			    //how to deal with subshells?
			case ';':
				if((!isLast) && semi_index == -1){
					semi_index = iter;
				}
				next_ch_ampe = false;
				next_ch_pipe = false;
				break;
			case '&':
				isLast = false;
				if( (andor_index == -1) && next_ch_ampe){
					andor_index = iter;
				}
				else {
					next_ch_ampe = true;
				}
				next_ch_pipe = false;
				break;
			case '|':
				isLast = false;
				if(pipe_index == -1){
					pipe_index = iter;
				}
				if( (andor_index == -1) && next_ch_pipe){
					andor_index = iter;
					pipe_index = -1;
				}
				else {
					next_ch_pipe = true;
				}
				next_ch_ampe = false;
				break;
			case ' ':
			case '\t':
				next_ch_ampe = false;
				next_ch_pipe = false;
				break;
			case '(':
				par_found = true;
				//do not break;
			default:
				isLast = false;
				next_ch_ampe = false;
				next_ch_pipe = false;
				break;
		}	
		iter = iter - 1;
	}
	
	//source = (struct command*) malloc(sizeof(struct command));
	if(source == NULL){ //printf("malloc failed...\n"); 
		return; 
	}

	if(semi_index != -1){ //semi colon found
		//printf("Above is a SEQUENCE (at position %d) command\n", semi_index);		
		source->type = SEQUENCE_COMMAND;
	
		char leftside[semi_index+1];
		char rightside[cmd_len - semi_index];
		strncpy(leftside, command, semi_index);
		leftside[semi_index] = '\0';
		strncpy(rightside, command + semi_index + 1, cmd_len - semi_index);

		source->u.command[0] = (struct command*) malloc(sizeof(struct command));
		source->u.command[1] = (struct command*) malloc(sizeof(struct command));
		add_command(leftside, source->u.command[0]);
		add_command(rightside, source->u.command[1]);
	}	
	else if(andor_index != -1){ //and or or found
		//printf("Above is an AND/OR (at position %d) command\n", andor_index);
		if(command[andor_index] == '&'){	
			source->type = AND_COMMAND; 
		}
		else{
			source->type = OR_COMMAND;
		}

		char leftside[andor_index+1];
		char rightside[cmd_len - andor_index - 1];
		strncpy(leftside, command, andor_index);
		leftside[andor_index] = '\0';
		strncpy(rightside, command + andor_index + 2, cmd_len - andor_index - 1);

		source->u.command[0] = (struct command*) malloc(sizeof(struct command));
		source->u.command[1] = (struct command*) malloc(sizeof(struct command));
		add_command(leftside, source->u.command[0]);
		add_command(rightside, source->u.command[1]);
	


	}
	else if(pipe_index != -1){ //pipe found
		//printf("Above is a PIPE (at position %d) command\n", pipe_index);
		source->type = PIPE_COMMAND;

		char leftside[pipe_index+1];
		char rightside[cmd_len - pipe_index];
		strncpy(leftside, command, pipe_index);
		leftside[pipe_index] = '\0';
		strncpy(rightside, command + pipe_index + 1, cmd_len - pipe_index);

		source->u.command[0] = (struct command*) malloc(sizeof(struct command));
		source->u.command[1] = (struct command*) malloc(sizeof(struct command));
		add_command(leftside, source->u.command[0]);
		add_command(rightside, source->u.command[1]);
	

	}
	else{
		//printf("Above is a SIMPLE or SUBSHELL command\n");
		if(par_found){
			source->type = SUBSHELL_COMMAND;
			size_t start_ = 0, finish_ = strlen(command) - 1;
			size_t i;
			for(i = 0; i < strlen(command); i++){
				if(command[i] == '('){
					start_ = i;
					break;
				}	
			}
			for( i = finish_; i  <= start_; i--){
				if(command[i] == ')'){
					finish_ = i;
					break;
				}
			}
			char temp[strlen(command)];
			strncpy(temp, command + start_ + 1, finish_ - start_ - 1);
			source->u.subshell_command = (command_t) malloc(sizeof(struct command));
			add_command(temp, source->u.subshell_command);
		}
		else{
			source->type = SIMPLE_COMMAND;
			char** ptr = (char**)malloc(sizeof(char*));

			int len = strlen(command);
			*ptr = (char*) malloc(len+1);
			bzero(*ptr, len+1);
			memcpy(*ptr, command, len);
			source->u.word = ptr;
			//printf("word:%s\n", temp_);
		}
	}
	source->status = -1; //not sure what to do here
	source->input = source->output = NULL;
	
	//
	//free(curr_cmd);
	//
};

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
	command_stream_t curr_stream = (command_stream_t) malloc(sizeof(struct command_stream));
	curr_stream->commands = (cmd_node*) malloc(sizeof(cmd_node));
	curr_stream->iterator = curr_stream->commands;
	char ch;
	char command[5000000];
	bzero(command,5000000);
	ch = (char)(*get_next_byte)(get_next_byte_argument);
	int par_cnt = 0; //zero means par's all closed
	bool cont_f = false; //flag for possibly cont if \n
	bool comment_f = false; //flag for comments
	do {
		int curr_size_ = strlen(command);
	/*	if((ch == ';') && !comment_f){
			command[curr_size_] = ch;
			cont_f = false;	
			if((par_cnt == 0)){
				//done with this command
				//only call if not empty string
				printf("THIS IS A COMMAND:\n%s\n***above was a command\n",command);
				bzero(command,5000000);
			}
		}
		else */
		if(ch == '\n'){
			if(comment_f){
				comment_f = false;
			}
			if(cont_f || par_cnt != 0){
				command[curr_size_] = ch;
			}
			else {
				if((par_cnt == 0) && (curr_size_ > 0)){
					//done with this commandi
					curr_stream->iterator->next = (cmd_node*) malloc(sizeof(cmd_node));
					curr_stream->iterator = curr_stream->iterator->next;
					curr_stream->iterator->cmd = (command_t) malloc(sizeof(struct command));
					add_command(command, curr_stream->iterator->cmd);
					bzero(command,5000000);
				}
			}//empty string could be passed to kyi?
		}else { //not \n or ; so we need to add to command and adjust 
		if(!comment_f){
			switch(ch)
			{
				case '|':
					cont_f = true;//stuff
					break;
				case '&':
					if((curr_size_ > 0) && (command[curr_size_ - 1] == '&')){
						cont_f = true;
					}				
					break;
				case '(':
					par_cnt = par_cnt + 1; 
					break;
				case ')':
					par_cnt = par_cnt - 1;
					break;//dotn think you should change cont_f here???
				case ' ':
				case '\t':
					//do not modify anything
					break;
				case '#':
					comment_f = true;
					break;
				default:
				   	cont_f = false;
					break;
			}
		}
			if(!comment_f){
				command[curr_size_] = ch;
			}
		}
		//printf("newline detected, preceeding %c%c\n", command_[count-2], command_[count-1]);
		ch = (*get_next_byte)(get_next_byte_argument);
	} while (ch != EOF);

	if(strlen(command) > 0){
		//printf("THIS IS USED\n");
		//done with this command
		curr_stream->iterator->next = (cmd_node*) malloc(sizeof(cmd_node));
		curr_stream->iterator = curr_stream->iterator->next;
		curr_stream->iterator->cmd = (command_t) malloc(sizeof(struct command));
		add_command(command, curr_stream->iterator->cmd);
	} 
//printf("%s\n", command);
// error (1, 0, "command reading not yet implemented");
	curr_stream->iterator = curr_stream->commands->next;
	//printf("type:%d\n", (int)curr_stream->iterator->cmd->type);
	return curr_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
//keep calling make_command_stream until recieve negative number
//add the returned command_stream_t to the stream
//
//basically just return 1 command, and remove that
 // error (1, 0, "command reading not yet implemented");
if(s==NULL)
		return NULL;

	//iterates through the commands and sets the next iterator
	if(s->iterator!=NULL){
		command_t cmd;
		cmd = s->iterator->cmd;
		s->iterator = s->iterator->next;
		//printf("read_cs\n");
		return cmd;
	}
	return NULL;
}