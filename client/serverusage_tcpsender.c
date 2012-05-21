/*
//=============================================================================+
// File name   : serverusage_tcpsender.c
// Begin       : 2012-02-28
// Last Update : 2012-05-18
// Version     : 4.4.0
//
// Website     : https://github.com/fubralimited/ServerUsage
//
// Description : This program accept a text as input (from serverusage.ko) and
//               sends data to a remote server via TCP.
//
// Author: Nicola Asuni
//
// (c) Copyright:
//               Fubra Limited
//               Manor Coach House
//               Church Hill
//               Aldershot
//               Hampshire
//               GU12 4RQ
//				 UK
//               http://www.fubra.com
//               support@fubra.com
//
// License:
//    Copyright (C) 2012-2012 Fubra Limited
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//    See LICENSE.TXT file for more information.
//=============================================================================+
*/

// TO COMPILE:
// gcc -O3 -g -pipe -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector -fno-strict-aliasing -fwrapv -fPIC --param=ssp-buffer-size=4 -D_GNU_SOURCE -o serverusage_tcpsender.bin serverusage_tcpsender.c

// USAGE EXAMPLE:
// staprun -b 1024 serverusage_client.ko smp=60 srvid="host001" | ./serverusage_tcpsender.bin "127.0.0.1" 9930 "/var/log/serverusage_cache.log"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * Size of buffer used to scan input data.
 */
#define BUFLEN 512

/**
 * Character used to mark a processed line on cache file.
 */
#define LMARK 35 // '#' character

/**
 * Append the input string on a log file
 * @param s string to append on log.
 */
void appendlog(const char *s, const char *file) {
	FILE *fp = NULL;
	int ret = EOF;
	fp = fopen(file, "a");
	if (fp != NULL) {
		ret = fputs(s, fp);
		fclose(fp);
	}
	if (ret == EOF) {
		// output an error message
		perror("ServerUsage-Client (appendlog)");
	}
}

/**
 * Main function.
 * @param argc Argument counter.
 * @param argv[] Array of arguments: IP address and Port number.
 */
int main(int argc, char *argv[]) {

	// file pointer
	char *ch = NULL;

	// structure containing an Internet socket address for server: an address family (always AF_INET for our purposes), a port number, an IP address
	struct sockaddr_in si_server;

	// size of si_server
	int slen = sizeof(si_server);

	// socket
	int s;

	// decode arguments
	if (argc != 4) {
		perror("This program accept a text as input from serverusage_client.ko module and send and sends data via TCP to the specified IP:PORT.\nYou must provide 3 arguments: ip_address, port, local_cache_file\nFOR EXAMPLE:\n./serverusage_tcpsender.bin \"127.0.0.1\" 9930 \"/var/log/serverusage_cache.log\"");
		exit(1);
	}

	// set input values
	char *ipaddress = (char *)argv[1];
	int port = atoi(argv[2]);
	char *cachelog = (char *)argv[3];

	// buffer used for a single log line
	char buf[BUFLEN];

	// file pointer for local cache
	FILE *fp;

	// lenght of the string to send
	int blen = 0;

	// current file pointer position (used for cache file)
	long int cpos = 0;

	// temporary file pointer position (used for cache file)
	long int tpos = 0;

	// used to track errors when sending cached content
	int cerr = 0;

	// initialize the si_server structure filling it with binary zeros
	memset((char *) &si_server, 0, slen);

	// use internet address
	si_server.sin_family = AF_INET;

	// set the IP address we want to bind to.
	si_server.sin_addr.s_addr = inet_addr(ipaddress);

	// set the port to listen to, and ensure the correct byte order
	si_server.sin_port = htons(port);

	// initialize buffer
	memset(buf, 0, BUFLEN);

	// initialize socket
	s = -1;

	// forever
	while (1) {

		// read one line at time from stdin
		if (scanf("%512[^\n]s", &buf)) {

			if ((buf[0] == '@') && (buf[1] == '@')) {
				// valid line of data to be transmitted

				// add a newline character
				strcat(buf, "\n");

				// check for the socket
				if (s > 0) {

					// send line
					if (sendto(s, buf, strlen(buf), 0, NULL, 0) == -1) {

						// output an error message
						perror("ServerUsage-Client (sendto)");

						// log the file on local cache
						appendlog(buf, cachelog);

					} else { // the line has been successfully sent

						// try to send log files on cache (if any)
						fp = fopen(cachelog, "rwb+");

						if (fp != NULL) {
							// get starting line position
							cpos = ftell(fp);
							cerr = 0;
							// send the cache logs
							while (fgets(buf, BUFLEN, fp) != NULL) {
								// check for valid lines
								if ((buf[0] == '@') && ((blen = strlen(buf)) > 30)) {
									// send line
									if (sendto(s, buf, blen, 0, NULL, 0) == -1) {
										// output an error message
										perror("ServerUsage-Client (sendto)");
										// mark error
										cerr = 1;
										// exit from while loop
										break;
									} else {
										// store the starting line position
										tpos = cpos;
										// get next line position
										cpos = ftell(fp);
										// move pointer at the beginning of the sent line
										fseek(fp, tpos, SEEK_SET);
										// mark line as sent
										fputc(LMARK, fp);
										// restore file pointer position
										fseek(fp, cpos, SEEK_SET);
									}
								}
							}
							if (cerr == 0) {
								// remove the cache file
								remove(cachelog);
							}
							fclose(fp);
						}

					} // end of else - when sending is working

				} else { // we do not have a valid socket

					// log the file on local cache
					appendlog(buf, cachelog);
				}

			} else {

				if ((buf[0] == '@') && (buf[1] == 'S')) {
					// this line indicates a starting block of data: open a TCP connection

					// check if a previous socket exist
					if (s > 0) {
						// close socket
						close(s);
						s = -1;
					}

					// start of block of data : create a network socket.
					// AF_INET says that it will be an Internet socket.
					// SOCK_STREAM Provides sequenced, reliable, two-way, connection-based byte streams.
					if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
						// print an error message
						perror("ServerUsage-Client (socket)");
					} else {
						// establish a connection to the server
						if (connect(s, &si_server, slen) == -1) {
							close(s);
							s = -1;
							// print an error message
							perror("ServerUsage-Client (connect)");
						}
					}

				} else {

					if ((buf[0] == '@') && (buf[1] == 'E')) {
						// this line indicates an ending block of data: close the socket
						close(s);
						s = -1;
					}

				}
			}

			// skip newline characters
			while (scanf("%1[\n]s", &buf)) {}

		}

	}

	// free resources
	free(ch);
	free(ipaddress);
	free(cachelog);

	// close program and return 0
	return 0;
}

//=============================================================================+
// END OF FILE
//=============================================================================+
