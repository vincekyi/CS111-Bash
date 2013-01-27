#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int main (int argc, char **argv) {

	if(argc != 2)
    	return 1;	

    char* term = strtok(argv[1], " ");

    pid_t p = fork();
    if(p==0){ //child
    	char* file = term;
    	char** arr; 
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
    	sleep(2);
    	execvp(file, arr);
    	printf("command failed\n");
    }
    else{
    	int status;
    	printf("main process\n");
    	if(waitpid(p, &status, 0)<0) {
    		printf("error\n");
    		return 1;
    	}
    	if(WIFEXITED(status)) {
    		kill(p, SIGKILL);
    	}
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