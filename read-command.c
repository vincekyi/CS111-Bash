// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
static const int FORWARD = 1;
static const int BACKWARD = 0;
int LINE = 1;
int curr_line_count = 1;
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
//node that is part of a linked list
//the node holds a complete command and a pointer to the next command
struct cmd_n{
	command_t cmd;
	struct cmd_n* next;
};
typedef struct cmd_n cmd_node;

struct command_stream {
	cmd_node* commands;
	cmd_node* iterator;
};

//returns true of the given string is blank, false otherwise
bool isBlank(const char* str) {
	int i;
	int len = strlen(str);
	if(len == 0 || str == NULL)
		return true;
	for(i=0; i<len; i++) {
		if(str[i]!='\t' && str[i]!=' ' && str[i]!='\n')
			return false;
	}
	return true;
}

//removes whitespaces
int remove_ws(const char* str, int index, int dir) {
	int i;
	if(dir>0) //forward direction
	{
		for(i=index; i<(int)strlen(str); i++) {
			if(str[i] != ' ') {
				break;
			}
		}
	}
	else { //backward direction
		for(i=index; i >=0; i--) {
			if(str[i] != ' ') {
				break;
			}
		}
	}
	return i;
}



int countLinesAfter(const char* str, int start_index){
	if(str == NULL) return 0;
	int len = strlen(str);
	if(start_index < 0 || len <= start_index) return 0;
	int i = start_index;
	int nl_count = 0;
	while(i < len){
		if(str[i] == '\n') nl_count = nl_count + 1;
		i = i + 1;
	}
//	printf("number of newlines found: %d\nthis was the offending command: %s\n", nl_count, str);
	return nl_count;
}


//returns 0 if valid, else return the line in which error occurred
int isValid(const char* str, int line) {
  int i = 0;
  int result = line;
  int len = (int)strlen(str);
  int b_ws = remove_ws(str, len-1, BACKWARD);
  int f_ws = remove_ws(str, 0, FORWARD);
  //makes sure that the first char isnt '|' or '&'
  if(str[b_ws] == '|' || str[b_ws] == '&' ||
  	 str[b_ws] == '<' || str[b_ws] == '>' ||
  	 str[f_ws] == '|' || str[f_ws] == '&' ||
  	 str[f_ws] == '<' || str[f_ws] == '>')
    return result;
  for(i=0; i<len; i++) {
    //check for valid characters
    if(!(63<(int)str[i] && (int)str[i]<91) &&
       !(96<(int)str[i] && (int)str[i]<123)&& 
       !(47<(int)str[i] && (int)str[i]<58) && 
       str[i]!='#' && str[i]!=',' &&
       str[i]!='!' && str[i]!='%' && str[i]!='+' && 
       str[i]!='-' && str[i]!='.' && str[i]!='/' &&
       str[i]!=':' && str[i]!='^' && str[i]!='_' && 
       str[i]!='|' && str[i]!='&' && str[i]!='(' &&
       str[i]!=')' && str[i]!='<' && str[i]!='>' &&
       str[i]!='\n' && str[i]!=' ' && str[i]!='\t') {
      return result;
    }
    //cannot start with a & or | after a newline
    if(i-1<len && str[i-1]=='\n' && (str[i] == '|' || str[i] == '&' 
    	|| str[0] == '<' || str[0] == '>'))
      return result;

  	//check for single &
  	if(str[i]=='&' && (str[i+1]!='&' || str[i-1]!='&'))
  		return result;

  	//check for > < case
  	if(str[i]=='>' || str[i]=='<') {
  		int j;
  		for(j=i+1; j<len; j++) {
  			if(str[j]=='>' || str[j]=='<') 
  				break;
  		}
  		char temp[len];
  		strncpy(temp, str+i+1, j-i-1);
  		temp[j-i-1] = '\0';
  		if(isBlank(temp))
  			return result;

  	}
    //increment line number
    if(str[i]=='\n') {
      result++;
    }
  }
  return 0; //no error
}

//recursively frees a command
void free_cmd(command_t cmd) {
	
  //free input/output strings if they are not null
  if(cmd->input!=NULL){
    free(cmd->input);
	}
  if(cmd->output!=NULL){
    free(cmd->output);
}

  //check the command type
	//printf("type:%d", (int)cmd->type);
  switch (cmd->type) {
    case AND_COMMAND:
    case SEQUENCE_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    {
    	if(cmd->u.command[0]!=NULL || cmd->u.command[1]!=NULL) {
	        free_cmd(cmd->u.command[0]);
	        free_cmd(cmd->u.command[1]);
    	}
        break;
    }
    case SUBSHELL_COMMAND:
    	if(cmd->u.subshell_command!=NULL){
      		free_cmd(cmd->u.subshell_command);
  		}
      break;
    default: //free simple command
    {
    	if(cmd->u.word!=NULL) {
	      char** ptr = cmd->u.word;
	      char* str = *ptr;
		    if(ptr!=NULL) {
		      	if(str!=NULL)
		      		free(str);
			    free(ptr);
	  	    }
	  	}
      break;
    }
  }
    //free the command
  if(cmd!=NULL)
    free(cmd);
}

void cleanup(command_stream_t cs) {

  cs->iterator = cs->commands;
  if(cs->commands == NULL) {
    free(cs);
    return;
  }
  //iterate through the linked list of cmd_nodes
  cmd_node* it = cs->iterator;
  while(it) {
  	if(it->cmd!=NULL)
    	free_cmd(it->cmd);
    //save the next node and free the current node
    cmd_node* next_node = it->next;
    free(it); 
    it = next_node;
  }
  if(cs!=NULL)
  	free(cs); 
}

int check_input(char* str)
{
  int result = -1;
  unsigned int i;
  for(i=0; i < strlen(str); i++) {
    if(str[i] == '<') {
      result = (int)i;
      break;
  	}
  }
  return result;
}

int check_output(char* str)
{
  int result = -1;
  unsigned int i;
  for(i=0; i < strlen(str); i++) {
    if(str[i] == '>') {
      result = (int)i;
      break;
    }
  }
  return result;
}

void add_command(const char* command, command_t source, command_stream_t cs, bool from_make){
	int line_count = 0;
	int semi_index = -1, andor_index = -1, pipe_index = -1;
	int o_in = -1, c_in = -1;
	bool next_ch_pipe = false;
	bool next_ch_ampe = false;
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
				if(((!isLast) && semi_index == -1) && !(o_in<iter && iter<c_in)){
					semi_index = iter;
				}
				next_ch_ampe = false;
				next_ch_pipe = false;
				break;
			case '&':
				isLast = false;
				if(( (andor_index == -1) && next_ch_ampe) && !(o_in<iter && iter<c_in)
						){
					andor_index = iter;
				}
				else {
					next_ch_ampe = true;
				}
				next_ch_pipe = false;
				break;
			case '|':
				isLast = false;
				if(pipe_index == -1  && !(o_in<iter && iter<c_in)){
					pipe_index = iter;
				}
				if(( (andor_index == -1) && next_ch_pipe) && !(o_in<iter && iter<c_in))
					{
					andor_index = iter;
					pipe_index = -1;
				}
				else {
					next_ch_pipe = true;
				}
				next_ch_ampe = false;
				break;
			case '\n':
				if(from_make){
			//		line_count = line_count + 1;
					 LINE = LINE + 1; }
					line_count++;
				isLast = false;
			case ' ':
			case '\t':
				next_ch_ampe = false;
				next_ch_pipe = false;
				break;
			case ')':
				if(c_in==-1 || (o_in > iter))
				{
					c_in = iter;
					o_in = 0;
					int count_ = 1;
					int k = iter-1;
					for(; k >= 0; k--){
						if(command[k] == ')') count_ = count_ + 1;
						if(command[k] == '(') count_ = count_ - 1;
						if(count_ == 0){ 
							o_in = k;
							break;
						}
					}
				}

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

	source->status = -1; //not sure what to do here
	source->input = NULL;
	source->output = NULL;
	
	if(semi_index != -1 && !(o_in<semi_index && semi_index<c_in)){ //semi colon found
		//printf("Above is a SEQUENCE (at position %d) command\n", semi_index);		
		source->type = SEQUENCE_COMMAND;
	
		char leftside[semi_index+1];
		char rightside[cmd_len - semi_index];
		strncpy(leftside, command, semi_index);
		leftside[semi_index] = '\0';
		strncpy(rightside, command + semi_index + 1, cmd_len - semi_index);

		source->u.command[0] = (struct command*) malloc(sizeof(struct command));
		source->u.command[1] = (struct command*) malloc(sizeof(struct command));
		add_command(leftside, source->u.command[0], cs, false);
		add_command(rightside, source->u.command[1], cs, false);
	}	
	else if(andor_index != -1 && !(o_in<andor_index && andor_index<c_in)){ //and or or found
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
		//printf("Line:%d, command:%s\n right:%s\n left:%s\n", LINE, command, rightside, leftside);
		//printf("lenth of rightside is: %d", (int)strlen(rightside));
	//	printf("leftside is: %s\n THIS IS AN AND/OR\nrightside is: %s\n", leftside, rightside);
		source->u.command[0] = (struct command*) malloc(sizeof(struct command));
		source->u.command[1] = (struct command*) malloc(sizeof(struct command));
		add_command(leftside, source->u.command[0], cs, false);
		add_command(rightside, source->u.command[1], cs, false);
	


	}
	else if(pipe_index != -1 && !(o_in<pipe_index && pipe_index<c_in)){ //pipe found
		//printf("Above is a PIPE (at position %d) command\n", pipe_index);
		source->type = PIPE_COMMAND;

		char leftside[pipe_index+1];
		char rightside[cmd_len - pipe_index];
		strncpy(leftside, command, pipe_index);
		leftside[pipe_index] = '\0';
		strncpy(rightside, command + pipe_index + 1, cmd_len - pipe_index);

		source->u.command[0] = (struct command*) malloc(sizeof(struct command));
		source->u.command[1] = (struct command*) malloc(sizeof(struct command));
		add_command(leftside, source->u.command[0], cs, false);
		add_command(rightside, source->u.command[1], cs, false);
	

	}
	else{
		//printf("Above is a SIMPLE or SUBSHELL command\n");
		if(o_in != -1){
			source->type = SUBSHELL_COMMAND;
			//size_t i;
			int start_ = o_in;
			int finish_ = c_in;
			char temp[strlen(command)];
			bzero(temp, strlen(command));
			strncpy(temp, command + start_ + 1, finish_ - start_ - 1);
			source->u.subshell_command = (command_t) malloc(sizeof(struct command));
			//printf("subshell starts at %d, finishes at %d: %s\n", start_, finish_, temp);
			add_command(temp, source->u.subshell_command, cs, false);
		}
		else{
			source->type = SIMPLE_COMMAND;
			source->u.word = NULL;
			//check for validity
			int err = isValid(command, LINE);
		//	printf("line %d: %s\n", LINE, command);
			if(err!=0 || isBlank(command)) {
				if(err == 0)
				{
					//printf("command: %s\n", command);
					fprintf(stderr, "%d: Syntaax Error\n", LINE-line_count);
				}
				else
					fprintf(stderr, "%d: Syntaxx Error\n", err);
				cleanup(cs);
				exit(1);
			}

			//find leading whitespace
			int i;
			int wsc_ = 0;
			int len = strlen(command);
			for(i = 0; i < len; i++){
				if( command[i] == ' '  ||
				    command[i] == '\n' ||
				    command[i] == '\t' ){
					wsc_ = wsc_ + 1;	
				}
				else{
					break;
				}
			}
			int rwsc_ = 0;
			if(wsc_ < len){
				for(i = len -1; i >= 0; i--){
					if( command[i] == ' '  ||
					    command[i] == '\n' ||
					    command[i] == '\t' ){
						rwsc_ = rwsc_ + 1;	
					}
					else{
						break;
					}
				}
			}
			char* str = (char*) malloc(len - wsc_ - rwsc_ + 1);
			bzero(str, len - wsc_ - rwsc_ + 1);
			memcpy(str, command + wsc_, len - wsc_ - rwsc_);
		
			char* input = NULL; 
			char* output = NULL;

  			int in = check_input(str);
  			int out = check_output(str);
  			if(in>0 && out>0) {
  				input = (char*) malloc(out-in);
  				bzero(input, out-in);
  				//remove whitespaces
  				int f_ws = remove_ws(str, in+1, FORWARD);
  				int b_ws = remove_ws(str, out-1, BACKWARD);

      			memcpy(input, str+f_ws, b_ws-f_ws+1);
				source->input = input;

				int ws = remove_ws(str, out+1, FORWARD);
				output = (char*) malloc(strlen(str)-out);
      			memcpy(output, str+ws, strlen(str)-ws);
				source->output = output;
				//create a new command
				char** new_cmd = (char**)malloc(sizeof(char*));
				*new_cmd = (char*)malloc(in+1);
			  	bzero(*new_cmd, in+1);
			  	ws = remove_ws(str, in-1, BACKWARD);
			    memcpy(*new_cmd, str, ws+1);
			    free(str);
			    source->u.word = new_cmd;
			    //printf("input:%s\n", input);
			    //printf("output:%s\n", output);
  			}
			else if(in > 0) {
				input = (char*) malloc(strlen(str)-in);
				bzero(input, strlen(str)-in);
				//remove whitespaces
				int ws = remove_ws(str, in+1, FORWARD);
      			memcpy(input, str+ws, strlen(str)-ws);
				source->input = input;
				//create a new command
				char** new_cmd = (char**)malloc(sizeof(char*));
				*new_cmd = (char*)malloc(in+1);
			  	bzero(*new_cmd, in+1);
			  	ws = remove_ws(str, in-1, BACKWARD);
			    memcpy(*new_cmd, str, ws+1);
			    free(str);
			    source->u.word = new_cmd;
			    //printf("input:%s\n", input);
			}
			else if(out > 0) {
				output = (char*) malloc(strlen(str)-out);
				bzero(output, strlen(str)-out);
				//remove whitespaces
				int ws = remove_ws(str, out+1, FORWARD);
      			memcpy(output, str+ws, strlen(str)-ws);
				source->output = output;
				//create a new command
				char** new_cmd = (char**)malloc(sizeof(char*));
				*new_cmd = (char*)malloc(out+1);
			  	bzero(*new_cmd, out+1);
			  	ws = remove_ws(str, out-1, BACKWARD);
			    memcpy(*new_cmd, str, ws+1);
			    free(str);
			    source->u.word = new_cmd;
			    //printf("output:%s\n", output);
			}
			else 
			{
				char** ptr = (char**)malloc(sizeof(char*));
				*ptr = str;
				source->u.word = ptr;
				source->input = NULL;
				source->output = NULL;
				//printf("no input/output\n");
			}

			//printf("command:%s\n", *(source->u.word));
		}
	}
	if(from_make)
		LINE++;
	//
	//free(curr_cmd);
	//
}

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
				LINE = LINE + 1;
			}
			else if(cont_f || par_cnt != 0){
				command[curr_size_] = ch;
			}
			else if(!cont_f && isBlank(command)) {
				LINE = LINE + 1;
			}
			else {
				if((par_cnt == 0) && (curr_size_ > 0)){
					//done with this command
					curr_stream->iterator->next = (cmd_node*) malloc(sizeof(cmd_node));
					curr_stream->iterator = curr_stream->iterator->next;
					curr_stream->iterator->cmd = (command_t) malloc(sizeof(struct command));
					//LINE = LINE + 1;
					add_command(command, curr_stream->iterator->cmd, curr_stream, true);
					bzero(command,5000000);
				}
			}
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
		if(par_cnt!=0) {
			fprintf(stderr, "%d: Syntax Error\n", LINE);
			cleanup(curr_stream);
			exit(1);
		}
		curr_stream->iterator->next = (cmd_node*) malloc(sizeof(cmd_node));
		curr_stream->iterator = curr_stream->iterator->next;
		curr_stream->iterator->cmd = (command_t) malloc(sizeof(struct command));
		//LINE = LINE + 1;
		add_command(command, curr_stream->iterator->cmd, curr_stream, true);
	} 
//	printf("Total Lines: %d\n", LINE);
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
	//printf("Lines %d\n", LINE);
	return NULL;
}
