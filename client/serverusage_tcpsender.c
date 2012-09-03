/*
//=============================================================================+
// File name   : serverusage_tcpsender.c
// Begin       : 2012-02-28
// Last Update : 2012-08-09
// Version     : 6.3.1
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
// staprun -b 1024 serverusage_client.ko smp=60 srvid="host001" | ./serverusage_tcpsender.bin "127.0.0.1" "9930" "/var/log/serverusage_cache.log"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
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
	FILE *fpal = NULL;
	int ret = EOF;
	if ((fpal = fopen(file, "a")) != NULL) {
		ret = fputs(s, fpal);
		fclose(fpal);
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

	// decode arguments
	if (argc != 4) {
		perror("This program accept a text as input from serverusage_client.ko module and sends data via TCP to the specified IP:PORT.\n\
		You must provide 3 arguments: ip_address, port, local_cache_file\n\
		FOR EXAMPLE:\n\
		./serverusage_tcpsender.bin \"127.0.0.1\" \"9930\" \"/var/log/serverusage_cache.log\"");
		exit(1);
	}

	// set input values

	// the IP address of the listening remote log server
	char *ipaddress = (char *)argv[1];

	// the TCP	port of the listening remote log server
	char *port = (char *)argv[2];

	// the local cache file to temporarily store the logs when the TCP connection is not available
	char *cachelog = (char *)argv[3];

	// buffer used for a single log line
	char buf[BUFLEN];

	// initialize buffer
	memset(buf, 0, BUFLEN);

	// buffer used to read input data
	char *rawbuf = NULL;

	// length of the raw buffer
	size_t rblen = 0;

	// count read chars with getline
	int glen = 0;

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

	// socket
	int s = -1;

	// true option for setsockopt
	int opttrue = 1;

	// structures to handle address information
	struct addrinfo hints, *res, *aip;

	// initialize structure
	memset(&hints, 0, sizeof(hints));

	// set parameters
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// get address info
	if (getaddrinfo(ipaddress, port, &hints, &res) != 0) {
		perror("ServerUsage-Client (getaddrinfo)");
		exit(1);
	}

	// check if the program is in loop mode
	int loopcontrol = 0;

	// forever
	while (loopcontrol < 2) {

		rawbuf = NULL;
		rblen = 0;

		// read one line at time from stdin
		if (((glen = getline(&rawbuf, &rblen, stdin)) > 3) && (glen < BUFLEN)) {

			loopcontrol = 0;

			if ((rawbuf[0] == '@') && (rawbuf[1] == '@')) {
				// valid line of data to be transmitted

				// check for the socket
				if (s > 0) {

					// send line
					if (send(s, rawbuf, glen, 0) == -1) {

						// output an error message
						perror("ServerUsage-Client (sendto)");

						// log the file on local cache
						appendlog(rawbuf, cachelog);

					} else { // the line has been successfully sent

						// try to send log files on cache (if any)
						if ((fp = fopen(cachelog, "rwb+")) != NULL) {
							// get starting line position
							cpos = ftell(fp);
							cerr = 0;
							// send the cache logs
							while (fgets(buf, BUFLEN, fp) != NULL) {
								// check for valid lines
								if ((buf[0] == '@') && ((blen = strlen(buf)) > 30)) {
									// send line
									if (send(s, buf, blen, 0) == -1) {
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
						} else {
							if (fp != NULL) {
								clearerr(fp);
							}
						}

					} // end of else - when sending is working

				} else { // we do not have a valid socket

					// log the file on local cache
					appendlog(rawbuf, cachelog);
				}

			} else {

				if ((rawbuf[0] == '@') && (rawbuf[1] == 'S')) {
					// this line indicates a starting block of data: open a TCP connection

					// check if a previous socket exist
					if (s > 0) {
						// close socket
						close(s);
						s = -1;
					}

					// for each socket type
					for (aip = res; aip; aip = aip->ai_next) {

						// try to create a network socket.
						s = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);

						if (s < 0) {
							switch (errno) {
								case EAFNOSUPPORT:
								case EPROTONOSUPPORT: {
									// skip the errors until the last address family
									if (aip->ai_next) {
										continue;
									} else {
										// handle unknown protocol errors
										perror("ServerUsage-Client (socket)");
										break;
									}
								}
								default: {
									// handle other socket errors
									perror("ServerUsage-Client (socket)");
									break;
								}
							}
						} else {

							if (aip->ai_family == AF_INET6) {
								// set socket to listen only IPv6
								if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &opttrue, sizeof(opttrue)) == -1) {
									perror("ServerUsage-Client (setsockopt : IPPROTO_IPV6 - IPV6_V6ONLY)");
									continue;
								}
							}
							// make socket reusable
							if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opttrue, sizeof(opttrue)) == -1) {
								perror("ServerUsage-Client (setsockopt : SOL_SOCKET - SO_REUSEADDR)");
								continue;
							}

							// establish a connection to the server
							if (connect(s, aip->ai_addr, aip->ai_addrlen) == -1) {
								close(s);
								s = -1;
								// print an error message
								perror("ServerUsage-Client (connect)");
							}
						}
					}

				} else {

					if ((rawbuf[0] == '@') && (rawbuf[1] == 'E')) {
						// this line indicates an ending block of data: close the socket
						close(s);
						s = -1;
					}

				}
			}

			free(rawbuf);

		} // end getline

		++loopcontrol;

	} // end of while

	// free resources
	if (s > 0) {
		close(s);
	}

	// close program and return 0
	return 0;
}

//=============================================================================+
// END OF FILE
//=============================================================================+
