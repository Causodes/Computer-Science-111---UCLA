/*
Tian Ye
tianyesh@gmail.com
704931660
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>	
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

/**
References:
	https://linux.die.net/man/3/getopt
	https://linux.die.net/man/3/getopt_long
	https://www.gnu.org/software/libc/manual/html_node/Using-Getopt.html
	https://www.tutorialspoint.com/c_standard_library/c_function_signal.htm
	https://www.rapidtables.com/code/linux/gcc.html
*/

/**
Input Redirection Helper Function:
*/
void input_redirection(char* input) {
	int in_fd = open(input, O_RDONLY);
	
	if(in_fd >= 0) {
		close(0);
		dup(in_fd);
		close(in_fd);
	}
	else {
		fprintf(stderr, "Error %s, problem in opening file %s.\n", strerror(errno), input);
		exit(2);
	}
}

/**
Output Redirection Helper Function:
*/
void output_redirection(char* output) {
	int out_fd = creat(output, S_IRWXU);
	
	if(out_fd >= 0) {
		close(1);
		dup(out_fd);
		close(out_fd);
	}
	else {
		fprintf(stderr, "Error %s, problem in creating file %s.\n", strerror(errno), output);
		exit(3);
	}
}

/**
SIGSEGV Handler Helper Function:
*/
void seg_fault() {
	char* nptr = NULL;
	*nptr = 'a';
}

/**
SIGSEGV Handler Helper Function:
*/
void signal_handler(int signal_num){
	if(signal_num == SIGSEGV) {
		fprintf(stderr, "Error, Segmentation Fault.\n");
		exit(4);
	}
}

/**
Main Code Body:
*/
int main(int argc, char** argv) {
	int c = 0;
	int seg_flag = 0;
	char* input_file = NULL;
	char* output_file = NULL;

	static struct option long_options[] =
	{
		{"input",		required_argument, 0, 'i'},
		{"output",		required_argument, 0, 'o'},
		{"segfault",	no_argument,	   0, 's'},
		{"catch",		no_argument,	   0, 'c'}
	};
	
	while((c = getopt_long(argc, argv, "i:o:sc", long_options, NULL)) != -1) {
		switch(c) {
			case 'i':
				input_file = optarg;
				break;
			case 'o':
				output_file = optarg;
				break;
			case 's':
				seg_flag = 1;
				break;
			case 'c':
				signal(SIGSEGV, signal_handler);
				break;
			default:
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab0 --input filename --output filename [--segfault --catch]\n");
				exit(1);
				break;
		}
	}
		
	if(input_file) {
		input_redirection(input_file);
	}
	
	if(output_file) {
		output_redirection(output_file);
	}
	
	if(seg_flag == 1) {
		seg_fault();
	}

	char buffer[1];
	while (read(0, buffer, 1))
	{
		write(1, buffer, 1);
	}
	exit(0);
}