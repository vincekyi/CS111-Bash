// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "command-internals.h"
#include "command.h"
#include "string.h"


//node in the linked list of commands
struct cmd_n {
  //contains a pointer to the command and the pointer to the next node
  command_t cmd;
  struct cmd_n* next;
};

typedef struct cmd_n cmd_node;
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream {
  cmd_node* commands; //list of commands (head)
  cmd_node* iterator; //iterator to keep track of current iteration
};

command_t createCommand(char* cmd) {
  command_t newCmd;
  newCmd = (command_t) malloc(sizeof(struct command));
  return newCmd;
}

//return 0 if no errors, else returns the line number if there is an error
int check_consec(char* str) {
  int line = 1; //keep track of the line number
  unsigned int i;
  int len = strlen(str);
  for(i=0; i<len; i++) {
    switch(str[i]) {
      case '&':
        if(i+1<len && i+2<len &&
          str[i+1]=='&' && str[i+2]=='&') {
          return line;
        }
        //cannot have just one &
        if(i+1<len && str[i+1]!='&') {
          return line;
        }
        break;
      case '|':
        if(i+1<len && i+2<len &&
          str[i+1]=='|' && str[i+2]=='|') {
          return line;
        }
        break;
      case '<':
        if(i+1<len && str[i+1]=='<') {
          return line;
        }
        break;
      case '>':
        break;
      case '\n':
        line++;
        break;
    }
  }
  return 0;
}

void free_cmd(command_t cmd) {
  //free input/output strings if they are not null
  if(cmd->input)
    free(cmd->input);
  if(cmd->output)
    free(cmd->output);
  printf("type %d\n", cmd->type);
  //check the command type
  switch (cmd->type) {
    case AND_COMMAND:
    case SEQUENCE_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    {
        free_cmd(cmd->u.command[0]);
        free_cmd(cmd->u.command[1]);
        break;
    }
    case SUBSHELL_COMMAND:
      free_cmd(cmd->u.subshell_command);
      break;
    default: //free simple command
    {
      printf("here 5\n");
      char** ptr = cmd->u.word;
      char* str = *ptr;
      free(str);
      free(ptr);
      break;
    }
  }
    //free the command
    free(cmd);
}

void cleanup(command_stream_t cs) {

  cs->iterator = cs->commands;
  if(cs->commands == NULL) {
    free(cs);
    return;
  }
  printf("here 1\n");
  //iterate through the linked list of cmd_nodes
  cmd_node* it = cs->iterator;
  printf("here 2\n");
  while(it) {
    free_cmd(cs->commands->cmd);
    printf("here 3\n");
    //save the next node and free the current node
    cmd_node* next_node = it->next;
    free(it); 
    it = next_node;
  }
  free(cs); 
}

//finds the index of the command that has the highest precedence
//precedence level: 1. ';', 2. '||'' or '&&', 3. '|''
int first_prec_index(char* str) {
  bool second = false, third = false; //flags for precendence levels
  int result = -1;
  int i;
  for(i=0; i<strlen(str); i++)
  {
    switch(str[i]) {
      case ';':
      /* if the semicolon is at the end of the command
        if(i == (strlen(str) - 1))
          break;
      */
        return i;
      case '&':
        if(!second) {
          if((i+1)<strlen(str) && str[i+1] == '&') {
            second = true;
            result = i;
            i++;
          }
        break;
      case '|':
        if(!second) {
          if((i+1)<strlen(str) && str[i+1] == '|') {
            second = true;
            result = i;
            i++;
          }
          else {
            if(!third) {
              third = true;
              result = i;
            }
          }
        }
        break;
      }
    }
  }
  return result;
}


//returns 0 if valid, else return the line in which error occurred
int isValid(char* str, int line) {
  int i;
  int result = line;
  int len = (int)strlen(str);
  //makes sure that the first char isnt '|' or '&'
  if(str[0] == '|' || str[0] == '&')
    return result;
  for(i=0; i<len; i++) {
    //check for valid characters
    if(!(63<(int)str[i] && (int)str[i]<91) &&
       !(96<(int)str[i] && (int)str[i]<123)&& str[i]!='#' &&
       str[i]!='!' && str[i]!='%' && str[i]!='+' && 
       str[i]!='-' && str[i]!='.' && str[i]!='/' &&
       str[i]!=':' && str[i]!='^' && str[i]!='_' && 
       str[i]!='|' && str[i]!='&' && str[i]!='(' &&
       str[i]!=')' && str[i]!='<' && str[i]!='>' &&
       str[i]!='\n' && str[i]!=' ' && str[i]!='\t') {
      return result;
    }
    //cannot start with a & or | after a newline
    if(i-1<len && str[i-1]=='\n' && (str[i] == '|' || str[i] == '&'))
      return result;
    //increment line number
    if(str[i]=='\n') {
      result++;
    }
  }
  return 0; //no error
}

int main(int argc, char **argv)
{
  if(argc != 2)
    return 1;

  //create/set-up command_stream
  command_stream_t cs;
  cs = (command_stream_t) malloc(sizeof(struct command_stream));
  cs->commands = NULL;
  cs->iterator = NULL;
  //insert_into_stream(argv[1], cs);
  printf("%s\n", argv[1]);
  printf("error on line %d\n", isValid(argv[1], 1));
  cleanup(cs);
  return 0;
}
