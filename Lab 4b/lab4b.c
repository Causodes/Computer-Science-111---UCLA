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

/**
Main Program Routine:
*/
int main(int argc, char **argv) {
	// Handle execution argument
	int c = 0;
	static struct option long_options[] = {
		{"scale",	required_argument,	0,	's'},
		{"period",	required_argument, 	0,	'p'},
		{"log",		required_argument,	0,	'l'}
	};
	
	while((c = getopt_long(argc, argv, "s:p:l", long_options, NULL)) != -1) {
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
			default:
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab4b [--scale=scale] [--period=period] [--log=log_file]\n");
				exit(1);
				break;
		}
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
	pollfd[0].fd = STDIN_FILENO;
	pollfd[0].events = POLLIN | POLLHUP | POLLERR;
	
	while(1) {
		
		if(poll(pollfd, 1, 0) < 0) {
			fprintf(stderr, "Error, polling failed.\n");
			exit(1);
		}
		
		// Shutdown signal processing
		if(mraa_gpio_read(general_io)) {
			if(mraa_aio_close(analog_io) != MRAA_SUCCESS) {
				fprintf(stderr, "Error, could not close AIO.\n");
				exit(1);
			}
			if(mraa_gpio_close(general_io) != MRAA_SUCCESS) {
				fprintf(stderr, "Error, could not close GPIO.\n");
				exit(1);
			}
			
			struct timeval start;
			gettimeofday(&start, 0);
			formatted_time = localtime(&(start.tv_sec));
			
			fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
			if(log_flag) {
				fprintf(file, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
			}
			fclose(file);
			
			exit(0);
		}
		
		// Read from STDIN
		/*
		if(pollfd[0].revents & (POLLIN | POLLERR | POLLHUP)) {
			char buffer[25];
			fgets(buffer, 25, stdin);
			
			// Input processing
			if(strcmp(buffer, "OFF\n") == 0) {
				if(log_flag) {
					fprintf(file, "%s", buffer);
				}
				
				// Initiate shutdown
				if(mraa_aio_close(analog_io) != MRAA_SUCCESS) {
				fprintf(stderr, "Error, could not close AIO.\n");
				exit(1);
				}
				if(mraa_gpio_close(general_io) != MRAA_SUCCESS) {
					fprintf(stderr, "Error, could not close GPIO.\n");
					exit(1);
				}
				
				struct timeval start;
				gettimeofday(&start, 0);
				formatted_time = localtime(&(start.tv_sec));
				
				fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
				if(log_flag) {
					fprintf(file, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
				}
				fclose(file);
				
				exit(0);
			}
			
			else if(strcmp(buffer, "START\n") == 0) {
				if(log_flag) {
					fprintf(file, "%s", buffer);
				}
				start_stop = 1;
			}
			
			else if(strcmp(buffer, "STOP\n") == 0) {
				if(log_flag) {
					fprintf(file, "%s", buffer);
				}
				start_stop = 0;
			}
			
			else if(strcmp(buffer, "SCALE=F\n") == 0) {
				if(log_flag) {
					fprintf(file, "%s", buffer);
				}
				scale = 'F';
			}
			
			else if(strcmp(buffer, "SCALE=C\n") == 0) {
				if(log_flag) {
					fprintf(file, "%s", buffer);
				}
				scale = 'C';
			}
			
			else if(log_flag && strlen(buffer) > 4 && buffer[0] == 'L' && buffer[1] == 'O' && buffer[2] == 'G' && buffer[3] == ' ') {
				fprintf(file, "%s", buffer);
			}
			
			else if(strlen(buffer) > 7 && buffer[0] == 'P' && buffer[1] == 'E' && buffer[2] == 'R' && buffer[3] == 'I' && buffer[4] == 'O' && buffer[5] == 'D' && buffer[6] == '=') {
				if(log_flag) {
					fprintf(file, "%s", buffer);
				}
				
				unsigned int i = 8;
				while(i < strlen(buffer) && isdigit(buffer[i])) {
					i++;
				}
				//TODO: Check to see if extra characters after digit is invalid??
				if(i != strlen(buffer)) {
					i = 9;
				}
				i--;
				if(i != 8) {
					int new_period = 0;
					int p = 0;
					for(; i > 6; i--) {
						int num = buffer[i] - '0';
						new_period += num * pow(10, p);
						p++;
					}
					period = new_period;
				}
			}
			
			else {
				fprintf(stderr, "Error, invalid command %s", buffer);
				if(log_flag) {
					fprintf(file, "%s", buffer);
				}
			}
			fflush(file);
		}
		*/
		
		//Read from STDIN
        if (pollfd[0].revents & POLLIN)
        {
			char buffer[512];
			int i = 0;
			int offset = 0;
			
            int num_bytes = read(0, buffer, 512);
			
            if (num_bytes < 0) {
                fprintf(stderr, "Error, invalid read from STDIN.\n");
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
                        if (file != NULL)
                        {
                            fprintf(file, "OFF\n");
                            fflush(file);
                        }
						if(mraa_aio_close(analog_io) != MRAA_SUCCESS) {
							fprintf(stderr, "Error, could not close AIO.\n");
							exit(1);
						}
						if(mraa_gpio_close(general_io) != MRAA_SUCCESS) {
							fprintf(stderr, "Error, could not close GPIO.\n");
							exit(1);
						}

						struct timeval start;
						gettimeofday(&start, 0);
						formatted_time = localtime(&(start.tv_sec));

						fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
						if(log_flag) {
							fprintf(file, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
						}
						fclose(file);
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
						if(log_flag) {
							fprintf(file, "%s\n", buffer + offset);
						}
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
				
				// Initiate shutdown
				if(mraa_aio_close(analog_io) != MRAA_SUCCESS) {
				fprintf(stderr, "Error, could not close AIO.\n");
				exit(1);
				}
				if(mraa_gpio_close(general_io) != MRAA_SUCCESS) {
					fprintf(stderr, "Error, could not close GPIO.\n");
					exit(1);
				}
				
				struct timeval start;
				gettimeofday(&start, 0);
				formatted_time = localtime(&(start.tv_sec));
				
				fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
				if(log_flag) {
					fprintf(file, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
				}
				fclose(file);
				
				exit(1);
				break;
		}
		
		if (clock.tv_sec >= next && start_stop) {
			formatted_time = localtime(&(clock.tv_sec));
			fprintf(stdout, "%02d:%02d:%02d %.1f\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec, temperature);
			if(log_flag) {
				fprintf(file, "%02d:%02d:%02d %.1f\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec, temperature);
			}
			next = clock.tv_sec + period;
		}
	}
	
	// Initiate shutdown
	if(mraa_aio_close(analog_io) != MRAA_SUCCESS) {
	fprintf(stderr, "Error, could not close AIO.\n");
	exit(1);
	}
	if(mraa_gpio_close(general_io) != MRAA_SUCCESS) {
		fprintf(stderr, "Error, could not close GPIO.\n");
		exit(1);
	}
	
	struct timeval start;
	gettimeofday(&start, 0);
	formatted_time = localtime(&(start.tv_sec));
	
	fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
	if(log_flag) {
		fprintf(file, "%02d:%02d:%02d SHUTDOWN\n", formatted_time -> tm_hour, formatted_time -> tm_min, formatted_time -> tm_sec);
	}
	fclose(file);
	exit(0);
}