/*
 * Copyright © 2020-2021 Francois Marier VA7GPL (va7gpl@fmarier.org)
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

/* Configure the TNC on the Kenwood TM-D710 (and perhaps other transceivers
 * too...) */

/* A PG-5G cable is used to connect the operation panel to the computer */

/* You might want to adjust the default serial port speed
 * by tuning SERIAL_SPEED in the source code below.
 */

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* the speed on the serial port (default = 9600baud) */
#define SERIAL_SPEED B9600
#define MAX_COMMAND_LENGTH 128

#define VERSION "1.13"

int main(int argc, char *argv[]) {
  int dev;
  int flag_index; /* flag index in argv[] */
  int clflag;     /* holds codes for command line flags */
  unsigned long initmode_int = 0;
  int soft_flow = 0;
  int hard_flow = 0;
  unsigned long maxframe_int = 0;
  long paclen_int = -1;
  unsigned long baudrate_int = 0;
  long txdelay_int = -1;
  long band_int = -1;
  struct termios oldtio, newtio;
  const char band_0[] = "TN 0,0\r";
  const char band_a[] = "TN 2,0\r";
  const char band_b[] = "TN 2,1\r";
  char command_mycall[MAX_COMMAND_LENGTH];
  char command_maxframe[MAX_COMMAND_LENGTH];
  char command_paclen[MAX_COMMAND_LENGTH];
  char command_txdelay[MAX_COMMAND_LENGTH];
  const char command_tnc_off[] = "TC 1\r";
  const char command_soft_flow[] = "XFLOW ON\r";
  const char command_hard_flow[] = "XFLOW OFF\r";
  const char command_baudrate_1200[] = "HBAUD 1200\r";
  const char command_baudrate_9600[] = "HBAUD 9600\r";
  const char command_kiss_on[] = "KISS ON\r";
  const char command_restart[] = "RESTART\r";
  char *initmode = NULL;
  char *band = NULL;
  char *serial_port = NULL;
  char *maxframe = NULL;
  char *paclen = NULL;
  char *baudrate = NULL;
  char *txdelay = NULL;
  char *callsign = NULL;
  const struct option long_options[] = {
      {"maxframe", 1, 0, 'm'},      {"paclen", 1, 0, 'p'},
      {"hardware-flow", 0, 0, 'h'}, {"software-flow", 0, 0, 's'},
      {"baudrate", 1, 0, 'b'},      {"txdelay", 1, 0, 'd'},
      {"callsign", 1, 0, 'c'},      {"band", 1, 0, 'B'},
      {"serialport", 1, 0, 'S'},    {"version", 0, 0, 'V'},
      {"initmode", 1, 0, 'i'},      {NULL, 0, 0, 0}};

  for (;;) {
    clflag = getopt_long(argc, argv, "m:p:b:c:d:B:S:i:hsV", long_options,
                         &flag_index);
    if (clflag == -1) {
      break;
    }

    switch (clflag) {
    case 'V':
      printf("tmd710_tncsetup version %s\n", VERSION);
      return 0;
    case 'S':
      if (serial_port) {
        fprintf(stderr, "Error: multiple serial ports specified !\n");
        return -1;
      }
      serial_port = optarg;
      break;
    case 'B':
      if (band) {
        fprintf(stderr, "Error: multiple bands specified !\n");
        return -1;
      }
      band = optarg;
      band_int = strtol(band, NULL, 10);
      if (band_int < 0 || band_int > 1) {
        fprintf(stderr, "Error: invalid band parameter specified ! Valid "
                        "values are 0 for Band A or 1 for Band B.\n");
        return -1;
      }
      break;
    case 'h':
      if (soft_flow != 0) {
        fprintf(stderr, "Error: it is not possible to select both software "
                        "flow control and hardware flow control\n");
        return -1;
      }
      hard_flow = 1;
      break;
    case 's':
      if (hard_flow != 0) {
        fprintf(stderr, "Error: it is not possible to select both software "
                        "flow control and hardware flow control\n");
        return -1;
      }
      soft_flow = 1;
      break;
    case 'i':
      initmode = optarg;
      initmode_int = strtoul(initmode, NULL, 10);
      if (initmode_int > 2) {
        fprintf(stderr, "Error: invalid INITMODE parameter specified !\n");
        return -1;
      }
      break;
    case 'm':
      if (maxframe) {
        fprintf(stderr, "Error: multiple MAXFRAME parameter specified !\n");
        return -1;
      }
      maxframe = optarg;
      maxframe_int = strtoul(maxframe, NULL, 10);
      if (maxframe_int < 1 || maxframe_int > 7) {
        fprintf(stderr, "Error: invalid MAXFRAME parameter specified !\n");
        return -1;
      }
      break;
    case 'p':
      if (paclen) {
        fprintf(stderr, "Error: multiple PACLEN parameter specified !\n");
        return -1;
      }
      paclen = optarg;
      paclen_int = strtol(paclen, NULL, 10);
      if (paclen_int < 0 || paclen_int > 255) {
        fprintf(stderr, "Error: invalid PACLEN parameter specified !\n");
        return -1;
      }
      break;
    case 'b':
      if (baudrate) {
        fprintf(stderr, "Error: multiple HBAUD parameter specified !\n");
        return -1;
      }
      baudrate = optarg;
      baudrate_int = strtoul(baudrate, NULL, 10);
      if (baudrate_int != 1200 && baudrate_int != 9600) {
        fprintf(stderr, "Error: invalid HBAUD parameter specified !\n");
        return -1;
      }
      break;
    case 'd':
      if (txdelay) {
        fprintf(stderr, "Error: multiple TXDELAY parameter specified !\n");
        return -1;
      }
      txdelay = optarg;
      txdelay_int = strtol(txdelay, NULL, 10);
      if (txdelay_int < 0 || txdelay_int > 120) {
        fprintf(stderr, "Error: invalid TXDELAY parameter specified !\n");
        return -1;
      }
      break;
    case 'c':
      if (callsign) {
        fprintf(stderr, "Error: multiple MYCALL parameter specified !\n");
        return -1;
      }
      callsign = optarg;
      break;
    }
  }

  if (serial_port == NULL || band_int == -1) {
    printf("Configuration of the TNC on the Kenwood TM-D710\n\n");
    printf("You must supply at least two arguments: the band (0 for Band A, 1 "
           "for Band B) and the serial port\n\n");
    printf("Options available:\n");
    printf("                   -B, --band           the frequency band to use "
           "(0 for Band A or 1 for Band B)\n");
    printf("                   -S, --serialport     the serial port to use for "
           "communication with the operation panel\n\n");
    printf("                   -b, --baudrate       sets the HBAUD parameter "
           "(TNC speed, 1200 or 9600 baud)\n");
    printf("                   -m, --maxframe       sets the MAXFRAME "
           "parameter (1-7)\n");
    printf("                   -p, --paclen         sets the PACLEN parameter "
           "(0-255)\n");
    printf("                   -d, --txdelay        sets the TXDELAY parameter "
           "(0-120)\n");
    printf("                   -s, --software-flow  configure the TNC for "
           "software flow-control\n");
    printf("                   -h, --hardware-flow  configure the TNC for "
           "hardware flow-control\n");
    printf("                   -c, --callsign       configure the callsign\n");
    printf("                   -i,                  Initialize TNC (default is "
           "into Packet KISS mode)\n");
    printf("                                             1 : TNC off\n");
    printf("                                             2 : Packet command "
           "mode only\n");
    printf("                   -V, --version        display the version\n");
    return 1;
  }

  if (callsign != NULL) {
    if (snprintf(command_mycall, MAX_COMMAND_LENGTH, "MYCALL %s\r", callsign) <
        0) {
      printf("snprintf error !\n");
      return 1;
    }
  }

  if (maxframe_int != 0) {
    if (snprintf(command_maxframe, MAX_COMMAND_LENGTH, "MAXFRAME %lu\r",
                 maxframe_int) < 0) {
      printf("snprintf error !\n");
      return 1;
    }
  }

  if (paclen_int >= 0) {
    if (snprintf(command_paclen, MAX_COMMAND_LENGTH, "PACLEN %lu\r",
                 paclen_int) < 0) {
      printf("snprintf error !\n");
      return 1;
    }
  }

  if (txdelay_int >= 0) {
    if (snprintf(command_txdelay, MAX_COMMAND_LENGTH, "TXDELAY %lu\r",
                 txdelay_int) < 0) {
      printf("snprintf error !\n");
      return 1;
    }
  }

  dev = open(serial_port, O_RDWR | O_NOCTTY);
  if (dev < 0) {
    perror(serial_port);
    return 1;
  }

  if (tcgetattr(dev, &oldtio) == -1) { /* save current serial port settings */
    printf("tcgetattr error !\n");
    return 1;
  }
  memset(&newtio, 0, sizeof(newtio)); /* clear struct for new port settings */
  /* setup new serial port settings */
  newtio.c_cflag = SERIAL_SPEED | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR | ICRNL;
  newtio.c_oflag = 0;
  newtio.c_lflag = ICANON;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 1 character arrives */
  newtio.c_cc[VEOL] = 0;  /* '\0' */
  newtio.c_cc[VEOL2] = 0; /* '\0' */

  /* clean the line and activate the settings for the port */
  if (tcflush(dev, TCIFLUSH) == -1) {
    printf("tcflush error !\n");
    return 1;
  }
  if (tcsetattr(dev, TCSANOW, &newtio) == -1) {
    printf("tcsetattr error !\n");
    return 1;
  }

  if (write(dev, command_tnc_off, strlen(command_tnc_off)) == -1) {
    return 1;
  }
  sleep(1);

  /*shut off the TNC if requested */
  if (initmode_int == 1) {
    if (write(dev, band_0, strlen(band_0)) == -1) {
      return 1;
    }
    sleep(2);
    return 0;
  }

  /* D710 version 1.01,3179 needs 3-4 seconds to settle down.  It
     also restores it's previous speed and state so you need to let
     it settle before making any changes */

  if (band_int == 0) {
    if (write(dev, band_a, strlen(band_a)) == -1) {
      return 1;
    }
    sleep(3);
  } else if (band_int == 1) {
    if (write(dev, band_b, strlen(band_b)) == -1) {
      return 1;
    }
    sleep(3);
  }
  if (baudrate_int == 1200) {
    if (write(dev, command_baudrate_1200, strlen(command_baudrate_1200)) ==
        -1) {
      return 1;
    }
    sleep(1);
  } else if (baudrate_int == 9600) {
    if (write(dev, command_baudrate_9600, strlen(command_baudrate_9600)) ==
        -1) {
      return 1;
    }
    sleep(1);
  }
  if (callsign != NULL) {
    if (write(dev, command_mycall, strlen(command_mycall)) == -1) {
      return 1;
    }
    sleep(1);
  }
  if (maxframe_int != 0) {
    if (write(dev, command_maxframe, strlen(command_maxframe)) == -1) {
      return 1;
    }
    sleep(1);
  }
  if (paclen_int >= 0) {
    if (write(dev, command_paclen, strlen(command_paclen)) == -1) {
      return 1;
    }
    sleep(1);
  }
  if (soft_flow != 0) {
    if (write(dev, command_soft_flow, strlen(command_soft_flow)) == -1) {
      return 1;
    }
    sleep(1);
  } else if (hard_flow != 0) {
    if (write(dev, command_hard_flow, strlen(command_hard_flow)) == -1) {
      return 1;
    }
    sleep(1);
  }
  if (txdelay_int >= 0) {
    if (write(dev, command_txdelay, strlen(command_txdelay)) == -1) {
      return 1;
    }
    sleep(1);
  }
  if (initmode_int == 2) {
    sleep(1);
    return 0;
  }
  if (initmode_int == 0) {
    if (write(dev, command_kiss_on, strlen(command_kiss_on)) == -1) {
      return 1;
    }
    sleep(1);
    if (write(dev, command_restart, strlen(command_restart)) == -1) {
      return 1;
    }
    sleep(1);
  }

  /* restore previous serial port settings */
  if (tcsetattr(dev, TCSANOW, &oldtio) == -1) {
    printf("tcsetattr error !\n");
    return 1;
  }
  if (close(dev) == -1) {
    printf("close error !\n");
    return 1;
  }

  return 0;
}
