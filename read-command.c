// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <error.h>
#include <string.h>
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

//node in the linked list of commands
struct cmd_node {
	//contains a pointer to the command and the pointer to the next node
	command_t cmd;
	cmd_node* next;
};
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream {
	cmd_node* commands; //list of commands (head)
	cmd_node* iterator; //iterator to keep track of current iteration
};


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
	char ch_;
	char command_[5000000];
	bzero(command_,5000000);
	ch_ = (char)(*get_next_byte)(get_next_byte_argument);
	int count = 0;

	do {
		int last = strlen(command_);
		command_[last] = ch_; 
		if((ch_ == '\n') || (ch_ == ';')){
			if(count >= 2){
				int wsc = 0;
				while(command_[count - 1 - wsc] == ' ' && ((count - 1 - wsc) > 0 )){ //check cases.. not sure
					wsc = wsc + 1;
				}
				//check if actually command is done
				switch(command_[count - 1 - wsc])
				{
					case '|':
						//stuff
						;
						break;
					case '&':
						;
						break;
					case '(':
						; 
						break;
					case ')':
						;
						break;
					case '<':
						;
						break;
					case '>':
						;
						break;
					default:
					   	// command is done
						break;
				}
			}
 		}	
		//printf("newline detected, preceeding %c%c\n", command_[count-2], command_[count-1]);
		count = count + 1;
		ch_ = (*get_next_byte)(get_next_byte_argument);
	} while (ch_ != EOF);

 
	printf("%s\n", command_);
// error (1, 0, "command reading not yet implemented");
	return NULL;
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
		//return cmd;
	}
	return NULL;
}
