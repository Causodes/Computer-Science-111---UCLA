/**
Name: Tian Ye
Email: tianyesh@gmail.com
UID: 704931660
*/

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <getopt.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>
#include <mraa.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

/**
References:
	http://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/
	https://www.geeksforgeeks.org/fgets-gets-c-language/
	http://pubs.opengroup.org/onlinepubs/007908799/xsh/systime.h.html
	https://www.sanfoundry.com/c-program-illustrate-reading-datafile/
	https://iotdk.intel.com/docs/master/mraa/aio_8h.html
	https://iotdk.intel.com/docs/master/mraa/gpio_8h.html
*/

/**
Global Variables:
*/
char scale = 'F';
int period = 1;
int log_flag = 0;
FILE *file = NULL;

mraa_aio_context analog_io;
mraa_gpio_context general_io;

int start_stop = 1;
double temperature;

char * id = "";
char * hostname = "";
int sockfd;
struct sockaddr_in serv_addr;
struct hostent *server;
int port_number;

/**
Shut Down Helper Function:
*/
void shut_down(){
	if(mraa_aio_close(analog_io) != MRAA_SUCCESS) {
		fprintf(stderr, "Error, could not close AIO.\n");
		exit(1);
	}
	if(mraa_gpio_close(general_io) != MRAA_SUCCESS) {
		fprintf(stderr, "Error, could not close GPIO.\n");
		exit(1);
	}
	
	struct timeval start;
	struct tm* formatted_time;
	gettimeofday(&start, 0);
	formatted_time = localtime(&(start.tv_sec));
	
	char buffer[256];
	sprintf(buffer, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
	fprintf(stderr, "%s", buffer);
	dprintf(sockfd, "%s", buffer);
	if(log_flag){
		fprintf(file, "%s", buffer);
		fflush(file);
	}

	fclose(file);
}

/**
Main Program Routine:
*/
int main(int argc, char **argv) {
	// Handle execution argument
	int c = 0;
	static struct option long_options[] = {
		{"scale",	required_argument,	0,	's'},
		{"period",	required_argument, 	0,	'p'},
		{"log",		required_argument,	0,	'l'},
		{"id",		required_argument,	0,	'i'},
		{"host",	required_argument,	0,	'h'}
	};
	
	while((c = getopt_long(argc, argv, "s:p:l:i:h", long_options, NULL)) != -1) {
		switch(c) {
			case 's':
				if(strlen(optarg) == 1 && (optarg[0] == 'F' || optarg[0] == 'C')) {
					scale = optarg[0];
				}
				else {
					fprintf(stderr, "Error, not a recognized scale. Use F and C for Fahrenheit and Celsius, respectively.\n");
					exit(1);
				}
				break;
			case 'p':
				period = atoi(optarg);
				break;
			case 'l':
				log_flag = 1;
				file = fopen(optarg, "w");
				if(file == NULL) {
					fprintf(stderr, "Error creating file.\n");
					exit(1);
				}
				break;
			case 'i':
				id = optarg;
				break;
			case 'h':
				hostname = optarg;
				break;
			default:
				// TODO: Update Error Message
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab4c_tcp [--scale=scale] [--period=period] [--log=log_file] [--id=id] [--host=hostname]\n");
				exit(1);
				break;
		}
	}
	if(optind < argc) {
		port_number = atoi(argv[optind]);
		if(port_number < 0) {
			fprintf(stderr, "Error, invalid port.\n");
			exit(1);
		}
	}
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		fprintf(stderr, "Error, failed to open socket.\n");
		exit(1);
	}
	server = gethostbyname(hostname);
	if(server == NULL) {
		fprintf(stderr, "Error, could not connect to server at hostname %s.\n", hostname);
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
		fprintf(stderr, "Error connecting to client socket.\n");
		exit(1);
	}
	
	char id_buffer[20];
	//TODO: Check newline char
	sprintf(id_buffer, "ID=%s\n", id);
	fprintf(stderr, "%s", id_buffer);
	dprintf(sockfd, "%s", id_buffer);
	if(log_flag){
		fprintf(file, "%s", id_buffer);
		fflush(file);
	}
	
	analog_io = mraa_aio_init(1);
	if(analog_io == NULL) {
		fprintf(stderr, "Error, failed to initialize AIO.\n");
		mraa_deinit();
		exit(1);
	}
	
	general_io = mraa_gpio_init(60);
	if(general_io == NULL) {
		fprintf(stderr, "Error, failed to initialize GPIO.\n");
		mraa_deinit();
		exit(1);
	}
	mraa_gpio_dir(general_io, MRAA_GPIO_IN);
	
	struct timeval clock;
	struct tm* formatted_time;
	time_t next = 0;
	
	struct pollfd pollfd[1];
	pollfd[0].fd = sockfd;
	pollfd[0].events = POLLIN | POLLHUP | POLLERR;
	
	while(1) {
		
		if(poll(pollfd, 1, 0) < 0) {
			fprintf(stderr, "Error, polling failed.\n");
			exit(1);
		}
		
		//Read from server
        if (pollfd[0].revents & POLLIN)
        {
			char buffer[512];
			int i = 0;
			int offset = 0;
			
            int num_bytes = read(sockfd, buffer, 512);
			
            if (num_bytes < 0) {
                fprintf(stderr, "Error, invalid read from server.\n");
				exit(1);
			}
            else if (num_bytes == 0) {
                continue;
			}
			
            for (; i < num_bytes; i++)
            {
				// Reach end of line; process command
                if (buffer[i] == '\n')
                {
                    buffer[i] = '\0';
					
                    if (strcmp(buffer + offset, "OFF") == 0) {
                        if (log_flag)
                        {
                            fprintf(file, "%s\n", buffer + offset);
                        }
						shut_down();
						exit(0);
                    }
					
                    else if(strcmp(buffer + offset, "START") == 0) {
						if(log_flag) {
							fprintf(file, "%s\n", buffer + offset);
						}
						start_stop = 1;
					}
					
					else if(strcmp(buffer + offset, "STOP") == 0) {
						if(log_flag) {
							fprintf(file, "%s\n", buffer + offset);
						}
						start_stop = 0;
					}
					
					else if(strcmp(buffer + offset, "SCALE=F") == 0) {
						if(log_flag) {
							fprintf(file, "%s\n", buffer + offset);
						}
						scale = 'F';
					}
					
					else if(strcmp(buffer + offset, "SCALE=C") == 0) {
						if(log_flag) {
							fprintf(file, "%s\n", buffer + offset);
						}
						scale = 'C';
					}
					
					else if (strncmp(buffer + offset, "LOG ", 4) == 0) {
						fprintf(file, "%s\n", buffer + offset);
					}
					
					else if (strncmp(buffer + offset, "PERIOD=", 7) == 0) {
						if(log_flag) {
							fprintf(file, "%s\n", buffer + offset);
						}
						
						int j = 8 + offset;
						unsigned int k = abs(j);
						while(k < strlen(buffer) && isdigit(buffer[j])) {
							j++;
							k++;
						}
						if(buffer[j] != '\0') {
							j = 9 + offset;
						}
						j--;
						
						if(j != 8 + offset) {
							int new_period = 0;
							int p = 0;
							for(; j > 6 + offset; j--) {
								int num = buffer[j] - '0';
								new_period += num * pow(10, p);
								p++;
							}
							period = new_period;
						}
					}

                    else {
						fprintf(stderr, "Error, invalid command %s", buffer);
						if(log_flag) {
							fprintf(file, "%s\n", buffer + offset);
						}
					}
					//fprintf(file, "Offset: %d\nBuffer: %s\n", offset, buffer + offset);
                    offset = i + 1;
					fflush(file);
                }
            }
        }
		
		gettimeofday(&clock, 0);
		
		int raw_temperature = mraa_aio_read(analog_io);
		int B = 4275;
		double R = 1023.0 / ((double)raw_temperature) - 1.0;
		R *= 100000.0;
		
		switch(scale) {
			case 'C':
				temperature = 1.0 / (log(R/100000.0)/B + 1/298.15) - 273.15;
				break;
			case 'F':
				temperature = (1.0 / (log(R/100000.0)/B + 1/298.15) - 273.15) * 9.0 / 5.0 + 32;
				break;
			default:
				fprintf(stderr, "Error, invalid scale found for temperature.\n");
				shut_down();
				exit(1);
				break;
		}
		
		if (clock.tv_sec >= next && start_stop) {
			formatted_time = localtime(&(clock.tv_sec));
			char output_buffer[256];
			sprintf(output_buffer, "%02d:%02d:%02d %.1f\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec, temperature);
			fprintf(stderr, "%s", output_buffer);
			dprintf(sockfd, "%s", output_buffer);
			if(log_flag){
				fprintf(file, "%s", output_buffer);
				fflush(file);
			}
			next = clock.tv_sec + period;
		}
	}

	shut_down();
	exit(0);
}