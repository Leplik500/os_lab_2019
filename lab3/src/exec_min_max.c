#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <stdio.h>

int main(int argc, char **argv) {

	int pid = fork();

	if ( pid == 0 ) {
		execvp("./sequential_min_max", argv);
	}

	/* Put the parent to sleep for 2 seconds--let the child finished executing */
	wait(NULL);

	printf( "Finished executing the parent process\n"
	        " - the child won't get here--you will only see this once\n" );

	return 0;
}