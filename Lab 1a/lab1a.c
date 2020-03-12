#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

/**
References:
	http://man7.org/linux/man-pages/man3/termios.3.html
	https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rttcga.htm
	http://man7.org/linux/man-pages/man3/atexit.3.html
	https://pubs.opengroup.org/onlinepubs/009696799/functions/tcsetattr.html
	http://man7.org/linux/man-pages/man2/getpid.2.html
	http://man7.org/linux/man-pages/man2/fork.2.html
	https://www.geeksforgeeks.org/c-program-demonstrate-fork-and-pipe/
	https://linux.die.net/man/2/waitpid
	https://linux.die.net/man/3/execvp
*/

struct termios terminal_state;

// Maximum Input Buffer Size
int max_buffer_size = 512;

// CR and LF Codes
char cr_lf[2] = {'\r', '\n'};
char lf = '\n';

// Process ID
pid_t pid;

// Pipes
int fd1[2];	// Pipe from terminal to shell
int fd2[2]; // Pipe from shell to terminal

/**
Helper function to restore previous terminal state:
*/
void restore_terminal_state() {
	tcsetattr(0, TCSANOW, &terminal_state);
	
	// Close pipes
	close(fd1[0]);
	close(fd1[1]);
	close(fd2[0]);
	close(fd2[1]);
}

int main(int argc, char **argv) {	
	// Handle execution argument
	int c = 0;
	int shell_flag = 0;
	
	static struct option long_options[] = {
		{"shell",	no_argument,	0,	's'}
	};
	
	// Save current terminal state
	tcgetattr(0, &terminal_state);
	atexit(restore_terminal_state);
	
	// Make copy per lab specs
	struct termios terminal_state_copy;
	tcgetattr(0, &terminal_state_copy);
	
	// Change settings of copied state
	terminal_state_copy.c_iflag = ISTRIP;
	terminal_state_copy.c_oflag = 0;
	terminal_state_copy.c_lflag = 0;
	
	// Error checking for failed terminal initialization
	if(tcsetattr(0, TCSANOW, &terminal_state_copy) < 0) {
		fprintf(stderr, "Failed to initialize terminal.\n");
		exit(1);
	}
	
	while((c = getopt_long(argc, argv, "s", long_options, NULL)) != -1) {
		switch(c) {
			case 's':
				shell_flag = 1;
				break;
			default:
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab1a [--shell]\n");
				exit(1);
				break;
		}
	}
	
	// Pipe initialization
	if(pipe(fd1) == -1)
	{
		fprintf(stderr, "Pipe from terminal to shell failed.\n");
		exit(1);
	}
	if(pipe(fd2) == -1)
	{
		fprintf(stderr, "Pipe from shell to terminal failed.\n");
		exit(1);
	}
	
	if(shell_flag == 1) {
		pid = fork();
		
		// Error checking for failed forking
		if (pid < 0) {
			fprintf(stderr, "Forked process %s failed.\n", strerror(errno));
			exit(1);
		}
		
		// Process execution complete
		else if (pid == 0) {
			// Close unnecessary pipes
			close(fd1[1]);
			close(fd2[0]);

			// Copy file descriptors to STDIN and STDOUT
			dup2(fd1[0], 0);
			dup2(fd2[1], 1);

			// Close remaining pipes
			close(fd1[0]);
			close(fd2[1]);
			
			// Terminal processing
			char path[] = "/bin/bash";
			char *args[2] = {path, NULL};
			if(execvp(path, args) == -1) {
				fprintf(stderr, "Cannot start new terminal session.\n");
				exit(1);
			}
		}
		
		// Run process
		else {
			// Create and array of two pollfd structures
			struct pollfd pollfds[2];
			
			// Close unnecessary pipes
			close(fd1[0]);
			close(fd2[1]);
			
			// STDIN pollfd
			pollfds[0].fd = 0;
			// Shell pollfd
			pollfds[1].fd = fd2[0];
		
			// Have both pollfds wait for either input or error
			pollfds[0].events = POLLIN | POLLHUP | POLLERR;
			pollfds[1].events = POLLIN | POLLHUP | POLLERR;
			
			char buffer[max_buffer_size];
			int ret_val;
			while(1) {
				ret_val = poll(pollfds, 2, 0);

				// Error checking for polling failure
				if(ret_val == -1) {
					fprintf(stderr, "Polling failure.\n");
					exit(1);
				}
				
				// STDIN pollfd is taking input (POLLIN)
				if((pollfds[0].revents & POLLIN)) {
					int read_byte_amount = read(0, &buffer, max_buffer_size);
					
					// Error checking
					if(read_byte_amount < 0) {
						fprintf(stderr, "Read failure in keyboard poll.\n");
						exit(1);
					}
				
					// Process read bytes
					int i;
					for(i = 0; i < read_byte_amount; i++) {	
						switch(buffer[i]) {
							// Keyboard map <cr> or <lf> to <cr><lf>; shell map only to <lf>
							case '\r':
							case '\n':
								write(1, &cr_lf, 2);
								write(fd1[1], &lf, 1);
								continue;
							// Detect ^D signal
							case 0x04:
								close(fd1[1]);
								int status;
								// Error Checking
								if(waitpid(pid, &status, 0) == -1) {
									fprintf(stderr, "Error in changing state of child.\n");
									exit(1);
								}
								else {
									int retStatus = WEXITSTATUS(status);
									int retState = WTERMSIG(status);
									fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n\r", retState, retStatus);
									exit(0);
								}
								break;
							// Detect ^C signal
							case 0x03:
								kill(pid, SIGINT);
								break;
							// Write to buffer normally 1 byte at a time
							default:
								write(1, &buffer[i], 1);
								write(fd1[1], &buffer[i], 1);
								continue;
						}
					}
				}
				
				// Shell pollfd is taking input (POLLIN)
				if((pollfds[1].revents & POLLIN)) {
					int read_byte_amount = read(fd2[0], &buffer, max_buffer_size);
					
					// Error checking
					if(read_byte_amount < 0) {
						fprintf(stderr, "Read failure in shell poll.\n");
						exit(1);
					}
				
					// Process read bytes
					int i;
					for(i = 0; i < read_byte_amount; i++) {	
						switch(buffer[i]) {
							// Map <cr> or <lf> to <cr><lf>
							case '\r':
							case '\n':
								write(1, &cr_lf, 2);
								continue;
							// Write to buffer normally 1 byte at a time
							default:
								write(1, &buffer[i], 1);
						}
					}
				}
				
				if(pollfds[0].revents & (POLLHUP | POLLERR)) {
					int status;
					
					// Error Checking
					if(waitpid(pid, &status, 0) == -1) {
						fprintf(stderr, "Error in changing state of child.\n");
						exit(1);
					}
					else {
						int retStatus = WEXITSTATUS(status);
						int retState = WTERMSIG(status);
						fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n\r", retState, retStatus);
						break;
					}
				}

				if(pollfds[1].revents & (POLLHUP | POLLERR)) {
					int status;
					
					// Error Checking
					if(waitpid(pid, &status, 0) == -1) {
						fprintf(stderr, "Error in changing state of child.\n");
						exit(1);
					}
					else {
						int retStatus = WEXITSTATUS(status);
						int retState = WTERMSIG(status);
						fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n\r", retState, retStatus);
						break;
					}
					
					exit(0);
				}
			}
		}
	}
	else {
		// Read keyboard input into a buffer
		char buffer[max_buffer_size];
		while(1) {
			// Larger read to account for slower system
			int read_byte_amount = read(0, &buffer, max_buffer_size);
			
			// Error checking
			if(read_byte_amount < 0) {
				fprintf(stderr, "Read failure.\n");
				exit(1);
			}
			
			// Process read bytes
			else {
				int i;
				for(i = 0; i < read_byte_amount; i++) {	
					switch(buffer[i]) {
						// Map <cr> or <lf> to <cr><lf>
						case '\r':
						case '\n':
							write(1, &cr_lf, 2);
							continue;
						// Detect ^D signal
						case 0x04:
							// TODO: Shutdown processing? Should be handled by exit(0)
							exit(0);
							break;
						// Write to buffer normally 1 byte at a time
						default:
							write(1, &buffer[i], 1);
							continue;
					}
				}
			}
		}
	}
	
	// Close all pipes (is this necessary?)
	close(fd1[0]);
	close(fd1[1]);
	close(fd2[0]);
	close(fd2[1]);
	
	return 0;
}