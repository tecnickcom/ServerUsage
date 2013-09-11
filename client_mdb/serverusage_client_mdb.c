/*
//=============================================================================+
// File name   : serverusage_client_mdb.c
// Begin       : 2012-08-14
// Last Update : 2012-09-18
// Version     : 6.3.8
//
// Website     : https://github.com/fubralimited/ServerUsage
//
// Description : This program gets the users' usage statistics from MariaDB
//               database and send them to ServerUsage-Server.
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
//               UK
//               http://www.fubra.com
//               support@fubra.com
//
// License:
//		Copyright (C) 2012-2012 Fubra Limited
//
//		This program is free software; you can redistribute it and/or modify
// 		it under the terms of the GNU General Public License as published by
// 		the Free Software Foundation; version 2 of the License.
//
// 		This program is distributed in the hope that it will be useful,
// 		but WITHOUT ANY WARRANTY; without even the implied warranty of
// 		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// 		GNU General Public License for more details.
//
// 		You should have received a copy of the GNU General Public License along
// 		with this program; if not, write to the Free Software Foundation, Inc.,
// 		51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// 		See LICENSE.TXT file for more information.
//=============================================================================+
*/

// TO COMPILE (requires MariaDB-devel package):
// gcc -O3 $(mysql_config --cflags) -o serverusage_client_mdb.bin serverusage_client_mdb.c $(mysql_config --libs)

// USAGE EXAMPLES:
// ./serverusage_client_mdb.bin LOGSERVER_IP LOGSERVER_PORT DATABASE_HOST DATABASE_PORT DATABASE_USER DATABASE_PASSWORD SAMPLING_TIME SERVER_ID PATH_TCPSENDER PATH_TEMPLOG
// ./serverusage_client_mdb.bin 127.0.0.1 9930 localhost 3306 "root" "PASSWORD" 60 "host001" "/usr/bin/serverusage_tcpsender_mdb.bin" "/var/log/serverusage_cache.log"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <mysql.h>

/**
 * Macro to find the max of two numbers
 */
#ifndef max
	#define max(a,b) (((a)>(b))?(a):(b))
#endif

/**
 * String buffer length.
 */
#define BUFLEN 512

/**
 * Report error message and close the program.
 */
void diep(const char *s) {
	perror(s);
	exit(1);
}

/**
 * Daemonize this process.
 */
static void daemonize(void) {
	pid_t pid, sid;
	if (getppid() == 1) {
		// this is already a daemon
		return;
	}
	// fork off the parent process
	pid = fork();
	if (pid < 0) {
		exit(1);
	}
	// if we got a good PID, then we can exit the parent process
	if (pid > 0) {
		exit(0);
	}
	// at this point we are executing as the child process
	// change the file mode mask
	umask(0);
	// create a new SID for the child process
	sid = setsid();
	if (sid < 0) {
		exit(1);
	}
	// change the current working directory to prevents the current directory from being locked
	if ((chdir("/")) < 0) {
		exit(1);
	}
	// redirect standard files to /dev/null
	FILE *ignore;
	ignore = freopen( "/dev/null", "r", stdin);
	ignore = freopen( "/dev/null", "w", stdout);
	ignore = freopen( "/dev/null", "w", stderr);
}

/**
 * Main function.
 * @param argc Argument counter.
 * @param argv[] Array of arguments: port, max_connections, mysql_server, mysql_user, mysql_password, mysql_database, mysql_table.
 */
int main(int argc, char *argv[]) {

	// decode arguments
	if (argc != 11) {
		diep("This program collects users statistics for MariaDB database and send them to Serverusage-Server\n\
		You must provide 10 arguments: LOGSERVER_IP LOGSERVER_PORT DATABASE_HOST DATABASE_PORT DATABASE_USER DATABASE_PASSWORD SAMPLING_TIME SERVER_ID PATH_TCPSENDER PATH_TEMPLOG\n\
		FOR EXAMPLE:\n\
		./serverusage_client_mdb.bin 127.0.0.1 9930 \"localhost\" 3306 \"root\" \"PASSWORD\" 60 \"host001\" \"/usr/bin/serverusage_tcpsender_mdb.bin\" \"/var/log/serverusage_cache.log\"");
	}

	// set input values

	// IP of machine running ServerUsage-Server
	char *serverip = (char *)argv[1];
	// TCP port of machine running ServerUsage-Server
	char *serverport = (char *)argv[2];
	// MariaDB hostname
	char *dbserver = (char *)argv[3];
	// MariaDB port
	int dbport = atoi(argv[4]);
	// MariaDB user
	char *dbuser = (char *)argv[5];
	// MariaDB password
	char *dbpassword = (char *)argv[6];
	// sampling time in seconds
	int smp = atoi(argv[7]);
	// Server ID
	char *srvid = (char *)argv[8];
	// Full path to serverusage_tcpsender_mdb.bin
	char *tcpsender = (char *)argv[9];
	// Full path to temporary cache file
	char *cachefile = (char *)argv[10];

	// daemonize this program
	daemonize();

	// set command to send data to ServerUsage-Server
	char tcpcmd[BUFLEN];
	// clear the string
	memset(tcpcmd, 0, BUFLEN);
	// compose the command
	sprintf(tcpcmd, "%s \"%s\" \"%s\" \"%s\"", tcpsender, serverip, serverport, cachefile);

	// database query used to get statistics
	char statquery[] = "SELECT USER, CPU_TIME, BYTES_SENT, BYTES_RECEIVED FROM information_schema.USER_STATISTICS";

	// MySQL variables
	MYSQL *dbconn;
	MYSQL_RES *dbresult;
	MYSQL_ROW dbrow;
	int db_num_fields;

	// initialize database conenction
	dbconn = mysql_init(NULL);

	// connect to database
	if (!mysql_real_connect(dbconn, dbserver, dbuser, dbpassword, NULL, dbport, NULL, 0)) {
		diep(mysql_error(dbconn));
	}

	// get CPU ticks per second
    long tps = sysconf(_SC_CLK_TCK);

	time_t starttime;
	time_t readtime;
	double timediff;

	// get starting time;
	starttime = time(NULL);

	// file pointer used to pipe output to serverusage_tcpsender_mdb.bin
	FILE *fp;

	while(1) {
		// get current time in seconds since EPOCH
		readtime = time(NULL);
		// time elapsed since last output
		timediff = difftime(readtime, starttime);
		// print data only at specific timestamps (to keep sampling window synchronized with scripts running on different servers)
		if ((timediff > 0) && (((long)readtime % smp) == 0)) {
			// check for the correct sampling interval
			if (timediff == smp) {
				// execute query to get stats
				if (!(mysql_query(dbconn, statquery))) {
					if ((dbresult = mysql_store_result(dbconn)) != NULL) {
						if ((db_num_fields = mysql_num_fields(dbresult)) == 4) {
							// open the pipe
							if ((fp = popen(tcpcmd, "w")) != NULL) {
								// mark the start of this block of data with an empty line
								fputs("@S\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\n", fp);
								while (dbrow = mysql_fetch_row(dbresult)) {
									if (dbrow[0] != NULL) {
										// NOTE: the UID is set to 0 and the user name is set to the IP field
										fprintf(fp, "@@\t%lu\t%lu\tmysql\t%s\t0\t%s\t%lu\t%lu\t%lu\t0\t0\n", (long)starttime, (long)readtime, srvid, dbrow[0], max(0, (long)(atof(dbrow[1]) * tps)), max(0, atol(dbrow[2])), max(0, atol(dbrow[3])));
									}
								}
								// mark the end of this block of data with an empty line
								fputs("@E\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\n", fp);
								// close the pipe
								pclose(fp);
							}
						}
						// clear database results
						mysql_free_result(dbresult);
					}
				}
			}
			// flush stats table
			if (mysql_query(dbconn, "FLUSH USER_STATISTICS")) {
				diep(mysql_error(dbconn));
			}
			// update starting sampling time
			starttime = readtime;
			readtime = time(NULL);
			timediff = difftime(readtime, starttime);
			// wait
			sleep(smp - timediff - 1);
		}
	}

	// close database connection
	mysql_close(dbconn);

	// close program and return 0
	return 0;
}

//=============================================================================+
// END OF FILE
//=============================================================================+
