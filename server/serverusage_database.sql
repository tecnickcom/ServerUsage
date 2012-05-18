/*
//=============================================================================+
File name   : serverusage_database.sql
Begin       : 2012-02-14
Last Update : 2012-05-16
Version     : 4.2.0

Website     : https://github.com/fubralimited/ServerUsage

Description : SQL file for ServerUsage project.
Database    : SQLite

Author: Nicola Asuni

(c) Copyright:
			Fubra Limited
			Manor Coach House
			Church Hill
			Aldershot
			Hampshire
			GU12 4RQ
			UK
			http://www.fubra.com
			support@fubra.com

License:

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	 See LICENSE.TXT file for more information.
//=============================================================================+
*/

-- -----------------------------------------------------
-- Table log_raw
-- Store raw log data arriving from remote hosts.
-- -----------------------------------------------------
DROP TABLE IF EXISTS log_raw;
CREATE TABLE log_raw (
  lrw_host_ip TEXT NOT NULL,
  lrw_host_port INTEGER NOT NULL,
  lrw_start_time INTEGER NOT NULL,
  lrw_end_time INTEGER NOT NULL,
  lrw_process TEXT NOT NULL,
  lrw_server_id TEXT NOT NULL,
  lrw_user_id INTEGER NOT NULL DEFAULT 0,
  lrw_ip TEXT NOT NULL,
  lrw_cpu_ticks INTEGER NOT NULL DEFAULT 0,
  lrw_io_read INTEGER NOT NULL DEFAULT 0,
  lrw_io_write INTEGER NOT NULL DEFAULT 0,
  lrw_netin INTEGER NOT NULL DEFAULT 0,
  lrw_netout INTEGER NOT NULL DEFAULT 0
);

-- -----------------------------------------------------
-- Table log_agg_hst
-- Aggregate raw data for process, uid, ip.
-- -----------------------------------------------------
DROP TABLE IF EXISTS log_agg_hst;
CREATE TABLE log_agg_hst (
  lah_start_time INTEGER NOT NULL,
  lah_end_time INTEGER NOT NULL,
  lah_process TEXT NOT NULL,
  lah_user_id INTEGER NOT NULL DEFAULT 0,
  lah_ip TEXT NOT NULL,
  lah_cpu_ticks INTEGER NOT NULL DEFAULT 0,
  lah_io_read INTEGER NOT NULL DEFAULT 0,
  lah_io_write INTEGER NOT NULL DEFAULT 0,
  lah_netin INTEGER NOT NULL DEFAULT 0,
  lah_netout INTEGER NOT NULL DEFAULT 0
);

/*=============================================================================+
// END OF FILE
//============================================================================*/
