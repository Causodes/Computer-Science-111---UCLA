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
#include <ulimit.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <zlib.h>

/**
References:
	https://linux.die.net/man/3/ulimit
	https://unix.stackexchange.com/questions/80598/ulimit-rlimit-in-linux-are-they-the-same-thing
*/

struct termios terminal_state;

// Maximum Input Buffer Size
int max_buffer_size = 512;

// Character Codes
char cr_lf[2] = {'\r', '\n'};
char c = '\3';

// Socket
int sockfd;

// Socket Character Buffer
struct sockaddr_in serv_addr;

// Host Computer
struct hostent *server;

// Port that Server Accepts Connections
int port_number;

// Logfile
char* log_file = NULL;
int log_fd;

// Compression Streams
z_stream in_strm;
z_stream out_strm;

/**
Helper function to restore previous terminal state:
*/
void restore_terminal_state() {
	tcsetattr(0, TCSANOW, &terminal_state);
}

void close_pipes() {
	close(sockfd);
}

int main(int argc, char **argv) {	
	// Handle execution argument
	int c = 0;
	int log_flag = 0;
	int compression_flag = 0;
	
	static struct option long_options[] = {
		{"port",		required_argument,	0,	'p'},
		{"log",			required_argument, 	0,	'l'},
		{"compression",	no_argument,		0,	'c'}
	};
	
	while((c = getopt_long(argc, argv, "p:l:c", long_options, NULL)) != -1) {
		switch(c) {
			case 'p':
				port_number = atoi(optarg);
				break;
			case 'l':
				log_file = optarg;
				log_flag = 1;
				break;
			case 'c':
				compression_flag = 1;
				break;
			default:
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab1b-client --port=port_number [--log=log_file] [--compress]\n");
				exit(1);
				break;
		}
	}
	
	// Error checking for port argument inclusion
	if(port_number == -1) {
		fprintf(stderr, "Error, required argument --port not found.\n");
		exit(1);
	}
	
	// Client initialization
	
	// TCP Connection
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	// Error checking for socket initialization
	if(sockfd < 0) {
		fprintf(stderr, "Error creating stream.\n");
		exit(1);
	}
	
	// Set host to localhost
	server = gethostbyname("localhost");
	if(server == NULL) {
		fprintf(stderr, "Error, could not connect to server at hostname localhost.\n");
		exit(1);
	}
	
	// Initialize read buffer to 0
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	// Set address family
	serv_addr.sin_family = AF_INET;
	
	// Set fields in serv_addr
	bcopy((char *) server -> h_addr,
		(char *) &serv_addr.sin_addr.s_addr,
		server -> h_length);
	
	// Assign port; convert port to network byte order using htons()
	serv_addr.sin_port = htons(port_number);
	
	// Connect to server
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Error connecting to server.\n");
		exit(1);
	}
	
	// Create logfile
	if(log_file != NULL) {
		log_fd = creat(log_file, 0666);
		if(log_fd < 0) {
			fprintf(stderr, "Opening of logfile failed.\n");
			write(sockfd, &c, 1);
			exit(1);
		}
		
	}
	
	// Ulimit wraps setrlimit
	struct rlimit rlim = {10000, 10000};
	if(setrlimit(RLIMIT_FSIZE, &rlim)) {
		fprintf(stderr, "Error setting file size limit.\n");
		exit(1);
	}
	
	// Save current terminal state
	tcgetattr(0, &terminal_state);
	
	// At exit executes in stack fashion
	atexit(restore_terminal_state);
	atexit(close_pipes);
	
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
		
	// Create and array of two pollfd structures
	struct pollfd pollfds[2];
	
	// STDIN pollfd
	pollfds[0].fd = 0;
	// Socket pollfd
	pollfds[1].fd = sockfd;

	// Have both pollfds wait for either input or error
	pollfds[0].events = POLLIN | POLLHUP | POLLERR;
	pollfds[1].events = POLLIN | POLLHUP | POLLERR;
	
	unsigned char buffer[max_buffer_size];
	unsigned char zlib_buffer[max_buffer_size];
	
	while(1) {
		int ret_val;
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
			
			if(compression_flag == 1 && read_byte_amount > 0) {
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
				
				// Send compressed bytes to server
				write(sockfd, &zlib_buffer, max_buffer_size - out_strm.avail_out);
				
				// Log bytes sent out
				if(log_flag == 1 && read_byte_amount > 0) {
					dprintf(log_fd, "SENT %d bytes: ", max_buffer_size - out_strm.avail_out);
					write(log_fd, &zlib_buffer, max_buffer_size - out_strm.avail_out);
					write(log_fd, "\n", 1);
				}
				
				// Called to avoid memory leak
				deflateEnd(&out_strm);
			}
			
			int i;
			for(i = 0; i < read_byte_amount; i++) {
				// Keyboard map <cr> or <lf> to <cr><lf>
				if(buffer[i] == '\r' || buffer[i] == '\n') {
					// Echo input to client screen
					write(1, &cr_lf, 2);
					if(compression_flag == 0) {
						write(sockfd, &buffer[i], 1);
					}
				}
				// Write to buffer normally 1 byte at a time
				else {
					// Echo input to client screen
					write(1, &buffer[i], 1);
					if(compression_flag == 0) {
						write(sockfd, &buffer[i], 1);
					}
				}
			}
			
			// Log bytes sent out
			if(log_flag == 1 && compression_flag == 0) {
				dprintf(log_fd, "SENT %d bytes: ", read_byte_amount);
				write(log_fd, buffer, read_byte_amount);
				write(log_fd, "\n", 1);
			}
			
		}
		
		// Socket is taking input (POLLIN)
		if((pollfds[1].revents & POLLIN)) {
			int read_byte_amount = read(sockfd, &buffer, max_buffer_size);
			
			// Error checking
			if(read_byte_amount < 0) {
				fprintf(stderr, "Read failure in shell poll.\n");
				close(sockfd);
				exit(1);
			}
			
			// Will this exit condition cause errors?
			if(read_byte_amount == 0) {
				exit(0);
			}
			
			// Log bytes received
			if(log_flag == 1 && read_byte_amount > 0) {
				dprintf(log_fd, "RECEIVED %d bytes: ", read_byte_amount);
				write(log_fd, buffer, read_byte_amount);
				write(log_fd, "\n", 1);
			}
			
			if(compression_flag == 1 && read_byte_amount > 0) {
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
				
				// Called to avoid memory leak
				inflateEnd(&in_strm);
				
				// Process decompressed bytes
				int decompressed_byte_amount = max_buffer_size - in_strm.avail_out;
				int i;
				for(i = 0; i < decompressed_byte_amount; i++) {
					switch(zlib_buffer[i]) {
						// Map <cr> or <lf> to <cr><lf>
						case '\r':
						case '\n':
							write(1, &cr_lf, 2);
							continue;
						// Write to buffer normally 1 byte at a time
						default:
							write(1, &zlib_buffer[i], 1);
					}
				}
			}
			
			else {
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
		}
		
		if(pollfds[0].revents & (POLLHUP | POLLERR)) {
			fprintf(stderr, "Server Connection Ended.\n");
			exit(0);
		}

		if(pollfds[1].revents & (POLLHUP | POLLERR)) {
			fprintf(stderr, "Server Connection Ended.\n");
			exit(0);
		}
	}
	return 0;
}