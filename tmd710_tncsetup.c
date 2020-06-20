/*
 * Copyright © 2010-2018 Guido Trentalancia IZ6RDB (iz6rdb@trentalancia.com)
 * Copyright © 2010 David Ranch KI6ZHD (dranch@trinnet.net)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Configure the TNC on the Kenwood TM-D710 (and perhaps other transceivers too...) */

/* A PG-5G cable is used to connect the operation panel to the computer */

/* You might want to adjust the default serial port speed
 * by tuning SERIAL_SPEED in the source code below.
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <getopt.h>

#define SERIAL_SPEED            B9600    /* the speed on the serial port (default = 9600baud) */
#define MAX_COMMAND_LENGTH      128

#define VERSION			"1.12"

int main(int argc, char *argv[]) {
	int dev;
	int flag_index; /* flag index in argv[] */
	int clflag; /* holds codes for command line flags */
	int initmode_int = 0;
	int soft_flow = 0;
	int hard_flow = 0;
	int maxframe_int = 0;
	int paclen_int = -1;
	int baudrate_int = 0;
	int txdelay_int = -1;
	int band_int = -1;
	struct termios oldtio, newtio;
	char band_0[7] = "TN 0,0\r";
	char band_a[7] = "TN 2,0\r";
	char band_b[7] = "TN 2,1\r";
	char *command_mycall;
	char *command_maxframe;
	char *command_paclen;
	char *command_txdelay;
	char command_tnc_off[5] = "TC 1\r";
	char command_soft_flow[9] = "XFLOW ON\r";
	char command_hard_flow[10] = "XFLOW OFF\r";
	char command_baudrate_1200[11] = "HBAUD 1200\r";
	char command_baudrate_9600[11] = "HBAUD 9600\r";
	char command_kiss_on[8] = "KISS ON\r";
	char command_restart[8] = "RESTART\r";
	char *initmode = NULL;
	char *band = NULL;
	char *serial_port = NULL;
	char *maxframe = NULL;
	char *paclen = NULL;
	char *baudrate = NULL;
	char *txdelay = NULL;
	char *callsign = NULL;
        const struct option long_options[] = {
                {"maxframe", 1, 0, 'm'},
                {"paclen", 1, 0, 'p'},
                {"hardware-flow", 0, 0, 'h'},
                {"software-flow", 0, 0, 's'},
		{"baudrate", 1, 0, 'b'},
		{"txdelay", 1, 0, 'd'},
		{"callsign", 1, 0, 'c'},
		{"band", 1, 0, 'B'},
		{"serialport", 1, 0, 'S'},
                {"version", 0, 0, 'V'},
                {"initmode", 1, 0, 'i'},
                {NULL, 0, 0, 0}
        };

        while (1) {
                clflag = getopt_long(argc, argv, "m:p:b:c:d:B:S:i:hsV", long_options,
                                     &flag_index);
                if (clflag == -1)
                        break;

                switch (clflag) {
                case 'V':
                        printf("tmd710_tncsetup version %s\n", VERSION);
                        exit(0);
                        break;
		case 'S':
			if (serial_port) {
				fprintf(stderr, "Error: multiple serial ports specified !\n");
				exit(-1);
			}
			serial_port = optarg;
			break;
                case 'B':
                        if (band) {
                                fprintf(stderr, "Error: multiple bands specified !\n");
                                exit(-1);
                        }
                        band = optarg;
			band_int = atoi(band);
			if (band_int < 0 || band_int > 1) {
				fprintf(stderr, "Error: invalid band parameter specified ! Valid values are 0 for Band A or 1 for Band B.\n");
                                exit(-1);
			}
                        break;
		case 'h':
			if (soft_flow) {
				fprintf(stderr, "Error: it is not possible to select both software flow control and hardware flow control\n");
				exit(-1);
			}
			hard_flow = 1;
			break;
		case 's':
			if (hard_flow) {
				fprintf(stderr, "Error: it is not possible to select both software flow control and hardware flow control\n");
				exit(-1);
			}
			soft_flow = 1;
			break;
		case 'i':
			initmode = optarg;
			initmode_int = atoi(initmode);
			if (initmode_int < 0 || initmode_int > 2) {
				fprintf(stderr, "Error: invalid INITMODE parameter specified !\n");
				exit(-1);
			}
			break;
		case 'm':
                        if (maxframe) {
                                fprintf(stderr, "Error: multiple MAXFRAME parameter specified !\n");
                                exit(-1);
                        }
                        maxframe = optarg;
			maxframe_int = atoi(maxframe);
			if (maxframe_int < 1 || maxframe_int > 7) {
				fprintf(stderr, "Error: invalid MAXFRAME parameter specified !\n");
				exit(-1);
			}
                        break;
                case 'p':
                        if (paclen) {
                                fprintf(stderr, "Error: multiple PACLEN parameter specified !\n");
                                exit(-1);
                        }
                        paclen = optarg;
			paclen_int = atoi(paclen);
			if (paclen_int < 0 || paclen_int > 255) {
				fprintf(stderr, "Error: invalid PACLEN parameter specified !\n");
				exit(-1);
			}
                        break;
                case 'b':
                        if (baudrate) {
                                fprintf(stderr, "Error: multiple HBAUD parameter specified !\n");
                                exit(-1);
                        }
                        baudrate = optarg;
			baudrate_int = atoi(baudrate);
			if (baudrate_int != 1200 && baudrate_int != 9600) {
				fprintf(stderr, "Error: invalid HBAUD parameter specified !\n");
				exit(-1);
			}
                        break;
		case 'd':
			if (txdelay) {
				fprintf(stderr, "Error: multiple TXDELAY parameter specified !\n");
				exit(-1);
			}
			txdelay = optarg;
			txdelay_int = atoi(txdelay);
			if (txdelay_int < 0 || txdelay_int > 120) {
				fprintf(stderr, "Error: invalid TXDELAY parameter specified !\n");
				exit(-1);
			}
			break;
		case 'c':
			if (callsign) {
				fprintf(stderr, "Error: multiple MYCALL parameter specified !\n");
				exit(-1);
			}
			callsign = optarg;
			break;
                }
        }

        if (serial_port == NULL || band_int == -1) {
                printf("Configuration of the TNC on the Kenwood TM-D710\n");
                printf("You must supply at least two arguments: the band (0 for Band A, 1 for Band B) and the serial port\n");
                printf("Options available:\n");
                printf("                   -b, --baudrate       sets the HBAUD parameter (TNC speed, 1200 or 9600 baud)\n");
                printf("                   -m, --maxframe       sets the MAXFRAME parameter (1-7)\n");
                printf("                   -p, --paclen         sets the PACLEN parameter (0-255)\n");
                printf("                   -d, --txdelay        sets the TXDELAY parameter (0-120)\n");
                printf("                   -s, --software-flow  configure the TNC for software flow-control\n");
                printf("                   -h, --hardware-flow  configure the TNC for hardware flow-control\n");
		printf("                   -c, --callsign       configure the callsign\n");
		printf("                   -S, --serialport     the serial port to use for communication with the operation panel\n");
		printf("                   -B, --band           the frequency band to use (0 for Band A or 1 for Band B)\n");
		printf("                   -i,                  Initialize TNC (default is into Packet KISS mode)\n");
		printf("                                             1 : TNC off\n");
		printf("                                             2 : Packet command mode only\n");
                printf("                   -V, --version        display the version\n");
                exit(1);
        }

        if (callsign != NULL) {
                command_mycall = (char *) malloc(MAX_COMMAND_LENGTH * sizeof(char));
                if (command_mycall == 0) {
                        printf("malloc error !\n");
                        exit(1);
                }
                sprintf(command_mycall, "MYCALL %s\r", callsign);
        }

        if (maxframe_int) {
                command_maxframe = (char *) malloc(MAX_COMMAND_LENGTH * sizeof(char));
                if (command_maxframe == 0) {
                        printf("malloc error !\n");
                        exit(1);
                }
                sprintf(command_maxframe, "MAXFRAME %u\r", maxframe_int);
        }

	if (paclen_int >= 0) {
	        command_paclen = (char *) malloc(MAX_COMMAND_LENGTH * sizeof(char));
	        if (command_paclen == 0) {
        	        printf("malloc error !\n");
	                exit(1);
	        }
	        sprintf(command_paclen, "PACLEN %u\r", paclen_int);
	}

        if (txdelay_int >= 0) {
                command_txdelay = (char *) malloc(MAX_COMMAND_LENGTH * sizeof(char));
                if (command_txdelay == 0) {
                        printf("malloc error !\n");
                        exit(1);
                }
                sprintf(command_txdelay, "TXDELAY %u\r", txdelay_int);
        }

	dev = open(serial_port, O_RDWR | O_NOCTTY);
	if (dev < 0) { perror(serial_port); exit(1); }

        tcgetattr(dev, &oldtio); /* save current serial port settings */
        bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
        /* setup new serial port settings */
        newtio.c_cflag = SERIAL_SPEED | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR | ICRNL;
        newtio.c_oflag = 0;
        newtio.c_lflag = ICANON;

        newtio.c_cc[VTIME]    = 0; /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 1; /* blocking read until 1 character arrives */
        newtio.c_cc[VEOL]     = 0; /* '\0' */
        newtio.c_cc[VEOL2]    = 0; /* '\0' */

        /* clean the line and activate the settings for the port */
        tcflush(dev, TCIFLUSH);
        tcsetattr(dev, TCSANOW, &newtio);

	write(dev, command_tnc_off, 5);
	sleep(1);

        /*shut off the TNC if requested */
	if (initmode_int == 1) {
	  write(dev, band_0, 7);
          sleep(2);
          exit(0);
        }

        /* D710 version 1.01,3179 needs 3-4 seconds to settle down.  It
           also restores it's previous speed and state so you need to let
           it settle before making any changes */

	if (band_int == 0) {
		write(dev, band_a, 7);
		sleep(3);
	} else if(band_int == 1) {
		write(dev, band_b, 7);
		sleep(3);
	}
	if (baudrate_int == 1200) {
		write(dev, command_baudrate_1200, 11);
		sleep(1);
	} else if (baudrate_int == 9600) {
		write(dev, command_baudrate_9600, 11);
		sleep(1);
	}
	if (callsign != NULL) {
		write(dev, command_mycall, strlen(command_mycall));
		sleep(1);
	}
	if (maxframe_int) {
		write(dev, command_maxframe, strlen(command_maxframe));
		sleep(1);
	}
	if (paclen_int >= 0) {
		write(dev, command_paclen, strlen(command_paclen));
		sleep(1);
	}
	if (soft_flow) {
		write(dev, command_soft_flow, 9);
		sleep(1);
	} else if (hard_flow) {
		write(dev, command_hard_flow, 10);
		sleep(1);
	}
	if (txdelay_int >= 0) {
		write(dev, command_txdelay, strlen(command_txdelay));
		sleep(1);
	}
	if (initmode_int == 2) {
                sleep(1);
                exit (0);
        }
	if (initmode_int == 0) {
	        write(dev, command_kiss_on, 8);
        	sleep(1);
        	write(dev, command_restart, 8);
        	sleep(1);
        }

	tcsetattr(dev, TCSANOW, &oldtio); /* restore previous serial port settings */
	close(dev);

	if (callsign != NULL)
		free(command_mycall);
	if (maxframe_int)
		free(command_maxframe);
	if (paclen_int >= 0)
		free(command_paclen);
	if (txdelay_int >= 0)
		free(command_txdelay);

	exit(0);
}

