/*
Tian Ye
tianyesh@gmail.com
704931660
*/
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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zlib.h>

/**
References:
	http://man7.org/linux/man-pages/man2/socket.2.html
	https://www.geeksforgeeks.org/socket-programming-cc/
	http://www.linuxhowtos.org/c_c++/socket.htm
	https://www.zlib.net/zlib_how.html
*/

// Maximum Input Buffer Size
int max_buffer_size = 512;

// LF Code
char lf = '\n';

// Process ID
pid_t pid;

// Pipes
int fd1[2];	// Pipe from terminal to shell
int fd2[2]; // Pipe from shell to terminal

// Sockets
int server_sockfd;
int client_sockfd;

// Socket Character Buffers
struct sockaddr_in serv_addr, cli_addr;

// Address of Client
socklen_t clilen;

// Port that Server Accepts Connections
int port_number;

// Compression Streams
z_stream out_strm;
z_stream in_strm;

/**
Helper function to close all pipes and sockets
*/
void close_pipes() {
	close(fd1[0]);
	close(fd1[1]);
	close(fd2[0]);
	close(fd2[1]);
	close(server_sockfd);
	close(client_sockfd);
}

/**
Helper function to close shell terminal
*/
void close_shell() {
	int status;
	waitpid(pid, &status, 0);
	int retStatus = WEXITSTATUS(status);
	int retState = WTERMSIG(status);
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n\r", retState, retStatus);
	exit(0);
}

/**
Signal handler
*/
void signal_handler(int sig) {
	if(sig == SIGINT) {
		kill(pid, SIGINT);
		close_shell();
	}
	else if(sig == SIGPIPE) {
		close_shell();
	}
	else if(sig == SIGABRT) {
		int status;
		waitpid(pid, &status, 0);
		int retStatus = WEXITSTATUS(status);
		int retState = WTERMSIG(status);
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n\r", retState, retStatus);
	}
}


int main(int argc, char **argv) {
	// Listen for signals
	signal(SIGINT, signal_handler);
	signal(SIGPIPE, signal_handler);
	
	atexit(close_pipes);
	
	// Handle execution argument
	int c = 0;
	int compression_flag = 0;
	
	static struct option long_options[] = {
		{"port",		required_argument,	0,	'p'},
		{"compress",	no_argument,		0,	'c'}
	};
	
	while((c = getopt_long(argc, argv, "p:c", long_options, NULL)) != -1) {
		switch(c) {
			case 'p':
				port_number = atoi(optarg);
				break;
			case 'c':
				compression_flag = 1;
				break;
			default:
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab1b --port=port_number [--compress]\n");
				exit(1);
				break;
		}
	}
	
	// Error checking for port argument inclusion
	if(port_number == -1) {
		fprintf(stderr, "Error, required argument --port not found.\n");
		exit(1);
	}

	// Server initialization
	
	// TCP Connection
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	// Error checking for socket initialization
	if(server_sockfd < 0) {
		fprintf(stderr, "Error creating stream.\n");
		exit(1);
	}
	
	// Initialize read buffer to 0
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	// Set address family
	serv_addr.sin_family = AF_INET;
	
	// Assign port; convert port to network byte order using htons()
	serv_addr.sin_port = htons(port_number);
	
	// Set IP address of host (localhost)
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
	// Bind socket to address
	if(bind(server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Error binding server socket.\n");
		exit(1);
	}
	
	// Listen on socket for connections
	listen(server_sockfd, 5);
	
	// Block proccess until client connects to server
	clilen = sizeof(cli_addr);
	client_sockfd = accept(server_sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if(client_sockfd < 0) {
		fprintf(stderr, "Error on client socket accept.\n");
		exit(1);
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
	
	pid = fork();
	
	// Error checking for failed forking
	if (pid < 0) {
		fprintf(stderr, "Forked process %s failed.\n", strerror(errno));
		exit(1);
	}
	
	// Child Process
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
	
	// Parent Process
	else {
		// Create and array of two pollfd structures
		struct pollfd pollfds[2];
		
		// Close unnecessary pipes
		close(fd1[0]);
		close(fd2[1]);
		
		// Client input pollfd
		pollfds[0].fd = client_sockfd;
		// Shell pollfd
		pollfds[1].fd = fd2[0];
	
		// Have both pollfds wait for either input or error
		pollfds[0].events = POLLIN | POLLHUP | POLLERR;
		pollfds[1].events = POLLIN | POLLHUP | POLLERR;
		
		unsigned char buffer[max_buffer_size];
		unsigned char zlib_buffer[max_buffer_size];
		
		while(1) {
			// Checking status of child process
			int status;
			if(waitpid(pid, &status, WNOHANG) == -1) {
				fprintf(stderr, "Child process ended.\n");
				int retStatus = WEXITSTATUS(status);
				int retState = WTERMSIG(status);
				fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n\r", retState, retStatus);
				exit(1);
			}
			
			int ret_val;
			// Error checking for polling failure
			ret_val = poll(pollfds, 2, 0);
			if(ret_val == -1) {
				fprintf(stderr, "Polling failure.\n");
				exit(1);
			}
			
			// Client pollfd is taking input (POLLIN)
			if((pollfds[0].revents & POLLIN)) {
				// Read from client socket
				int read_byte_amount = read(client_sockfd, &buffer, max_buffer_size);
				
				// Error checking
				if(read_byte_amount < 0) {
					fprintf(stderr, "Read failure from client.\n");
					exit(1);
				}
				
				if(compression_flag == 1) {
					// Allocate inflate state
					in_strm.zalloc = Z_NULL;
					in_strm.zfree = Z_NULL;
					in_strm.opaque = Z_NULL;
					
					// Error checking
					int ret = inflateInit(&in_strm);
					if(ret != Z_OK) {
						fprintf(stderr, "Error, cannot allocate inflate state.\n");
						exit(1);
					}
					
					// Number of bytes
					in_strm.avail_in = read_byte_amount;
					
					// Pointer to input space
					in_strm.next_in = buffer;
					
					// Maximum size of output bytes
					in_strm.avail_out = max_buffer_size;
					
					// Pointer to output space
					in_strm.next_out = zlib_buffer;
					
					// Decompress into zlib_buffer
					do {
						ret = inflate(&in_strm, Z_SYNC_FLUSH);
						if(ret != Z_OK) {
							fprintf(stderr, "Decompression error, zlib error number %d.\n", ret);
							exit(1);
						}
					}
					while(in_strm.avail_in > 0);
					
					// Process decompressed bytes
					int decompressed_byte_amount = max_buffer_size - in_strm.avail_out;
					int i;
					for(i = 0; i < decompressed_byte_amount; i++) {
						switch(zlib_buffer[i]) {
							// Map shell to <lf>
							case '\r':
							case '\n':
								write(fd1[1], &lf, 1);
								continue;
							// Detect ^D signal
							case 0x04:
							// Detect ^C signal
							case 0x03:
								close_pipes();
								break;
							// Write to buffer normally 1 byte at a time
							default:
								write(fd1[1], &zlib_buffer[i], 1);
								continue;
						}
					}					
					// Called to avoid memory leak
					inflateEnd(&in_strm);
				}
				
				else {
					// Process read bytes
					int i;
					for(i = 0; i < read_byte_amount; i++) {	
						switch(buffer[i]) {
							// Map shell to <lf>
							case '\r':
							case '\n':
								write(fd1[1], &lf, 1);
								continue;
							// Detect ^D signal
							case 0x04:
								// Close writing pipe to stop input
								close(fd1[1]);
								close_shell();
								break;
							// Detect ^C signal
							case 0x03:
								kill(pid, SIGINT);
								break;
							// Write to buffer normally 1 byte at a time
							default:
								write(fd1[1], &buffer[i], 1);
								continue;
						}
					}
				}
			}
			
			// Shell pollfd is taking input (POLLIN)
			if((pollfds[1].revents & POLLIN)) {
				// Read from shell pipe
				int read_byte_amount = read(fd2[0], &buffer, max_buffer_size);
				
				// Error checking
				if(read_byte_amount < 0) {
					fprintf(stderr, "Read failure in shell poll.\n");
					exit(1);
				}
				
				if(compression_flag == 1) {
					// Allocate deflate state
					out_strm.zalloc = Z_NULL;
					out_strm.zfree = Z_NULL;
					out_strm.opaque = Z_NULL;
					
					// Error checking
					int ret = deflateInit(&out_strm, Z_DEFAULT_COMPRESSION);
					if(ret != Z_OK) {
						fprintf(stderr, "Error, cannot allocate deflate state.\n");
						exit(1);
					}
					
					// Number of bytes
					out_strm.avail_in = read_byte_amount;
					
					// Pointer to input space
					out_strm.next_in = buffer;
					
					// Maximum size of output bytes
					out_strm.avail_out = max_buffer_size;
					
					// Pointer to output space
					out_strm.next_out = zlib_buffer;
					
					// Decompress into zlib_buffer
					do {
						ret = deflate(&out_strm, Z_SYNC_FLUSH);
						if(ret != Z_OK) {
							fprintf(stderr, "Compression error, zlib error number %d.\n", ret);
							exit(1);
						}
					}
					while(out_strm.avail_in > 0);
					
					// Called to avoid memory leak
					deflateEnd(&out_strm);
					
					// Send compressed bytes to client
					write(client_sockfd, &zlib_buffer, max_buffer_size - out_strm.avail_out);
				}
				
				else {
					// Process read bytes
					int i;
					for(i = 0; i < read_byte_amount; i++) {
						// No character processing
						write(client_sockfd, &buffer[i], 1);
					}
				}
				
			}
			
			if(pollfds[0].revents & (POLLHUP | POLLERR)) {
				close_shell();
			}
		}
	}
	return 0;
}