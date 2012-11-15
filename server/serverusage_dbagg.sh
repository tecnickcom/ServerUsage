#!/bin/sh
#===============================================================================
# File name   : serverusage_dbagg.sh
# Begin       : 2012-02-14
# Last Update : 2012-08-08
# Version     : 6.3.4
#
# Website     : https://github.com/fubralimited/ServerUsage
#
# Description : SQL file aggregator for ServerUsage project.
# Database    : SQLite
#
# Author: Nicola Asuni
#
# (c) Copyright:
# 			Fubra Limited
# 			Manor Coach House
# 			Church Hill
# 			Aldershot
# 			Hampshire
# 			GU12 4RQ
# 			UK
# 			http://www.fubra.com
# 			support@fubra.com
#
# License:
#
# 	This program is free software: you can redistribute it and/or modify
# 	it under the terms of the GNU Affero General Public License as
# 	published by the Free Software Foundation, either version 3 of the
# 	License, or (at your option) any later version.
#
# 	This program is distributed in the hope that it will be useful,
# 	but WITHOUT ANY WARRANTY; without even the implied warranty of
# 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# 	GNU Affero General Public License for more details.
#
# 	You should have received a copy of the GNU Affero General Public License
# 	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# 	 See LICENSE.TXT file for more information.
#===============================================================================

# Consume the log_raw table by aggregating data on log_agg_hst table

# This script can be executed every 5 minutes via cron. For example:
# 2,7,12,17,22,27,32,37,42,47,52,57 * * * * sleep 30; /usr/bin/serverusage_dbagg.sh
# The minutes and sleep command on the above example are choosen to execute the SQL off-peak.

# configuration file
CONFIG_FILE="/etc/serverusage_server.conf"

# load parameters from configuration file
if [ -f $CONFIG_FILE ] ; then
	. $CONFIG_FILE
fi

# get current timestamp (UNIX EPOCH)
NOWTIME=`date +%s`

# set process time delay
PROCESSTIME=`expr $NOWTIME - $DB_AGGREGATION_DELAY`

# set time to delete old data (every 7 days)
GARBAGETIME=`expr $NOWTIME - $DB_GARBAGE_TIME`

# garbage collector for log_agg_hst table
SQLA="DELETE FROM log_agg_hst WHERE (lah_end_time <= $GARBAGETIME);"

# aggregate data on log_agg_hst table for process, uid, ip
# the special swapper process (process 0) is always excluded
SQLB="INSERT INTO log_agg_hst (lah_start_time, lah_end_time, lah_process, lah_user_id, lah_ip, lah_cpu_ticks, lah_io_read, lah_io_write, lah_netin, lah_netout) SELECT lrw_start_time, lrw_end_time, lrw_process, lrw_user_id, lrw_ip, SUM(lrw_cpu_ticks), SUM(lrw_io_read), SUM(lrw_io_write), SUM(lrw_netin), SUM(lrw_netout) FROM log_raw WHERE lrw_process!='swapper' AND (lrw_end_time <= $PROCESSTIME) GROUP BY lrw_start_time, lrw_end_time, lrw_process, lrw_user_id, lrw_ip;"

# delete processed rows from raw table
SQLC="DELETE FROM log_raw WHERE (lrw_end_time <= $PROCESSTIME);"

#execute SQL query
sqlite3 $SQLITE_DATABASE "BEGIN TRANSACTION;$SQLA$SQLB$SQLC;END TRANSACTION;"

#===============================================================================
# END OF FILE
#===============================================================================
