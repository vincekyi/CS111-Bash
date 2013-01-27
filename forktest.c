#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

int executeCommand(char* command, char* output) {
	char* term = strtok(command, " ");

    int fp = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    int orig = dup(1);
    dup2(fp, 1);
    //close(fp);
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

    	/*
    	int j;
	    for(j=0; j<i; j++) {
	    	printf("%s\n", arr[j]);
    	}
    	*/
    	//close(1);
    	sleep(1);
    	execvp(file, arr);
    }
    else{
    	close(fp);
    	dup2(orig, 1);
    	int status;
    	if(waitpid(p, &status, 0)<0) {
    		return -1;
    	}
    	if(WIFEXITED(status)) {
    		kill(p, SIGKILL);
    	}
    }
    return 0;
}


int main (int argc, char **argv) {

	if(argc != 2)
    	return 1;	
    char* output = "out";

   if(executeCommand(argv[1], output)==0) {
   		printf("Correct\n");
   }
   else {
   		printf("Incorrect\n");
   }


	/*
	pid_t p = fork();
	int* volatile ptr = &global;
	if(p==0) {
		*ptr = 1;
	}
	else {
		sleep(2);
		printf("global: %d\n", global);
	}
	*/
	return 0;
}