<?php
//=============================================================================+
// File name   : serverusage_api.php
// Begin       : 2012-03-12
// Last Update : 2012-05-17
// Version     : 5.1.0
//
// Website     : https://github.com/fubralimited/ServerUsage
//
// Description : Web service to extract and return information
//               from ServerUsage log_agg_hst database table.
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
//               http://www.rackmap.net
//               support@rackmap.net
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

/**
 * @file
 * Web service to extract and return information from ServerUsage log_agg_hst database table.
 * @package com.fubra.serverusage
 * @brief Web service to extract and return information from ServerUsage log_agg_hst database table.
 * @author Nicola Asuni
 * @since 2012-03-12
 */


/*

PARAMETERS:

	from: (integer) starting timestamp in seconds since EPOCH;
	to: (integer) starting timestamp in seconds since EPOCH;
	metric: type of info to extract; Possible values are: 'uid', 'ip', 'uip', 'grp', 'glb', 'all'. The return values for each metric are:
		uid : user_id, cpu_ticks;
		ip  : ip, net_in, net_out;
		uip : user_id, ip, cpu_ticks, io_read, io_write, net_in, net_out;
		grp : start_time, end_time, user_id, ip, cpu_ticks, io_read, io_write, net_in, net_out;
		glb : (default for SVG mode) lah_start_time, lah_end_time, lah_cpu_ticks, lah_io_read, lah_io_write, lah_netin, lah_netout;
		all : start_time, end_time, process, user_id, ip, cpu_ticks, io_read, io_write, net_in, net_out.
	uid : (integer) if set, filter result for the requested user ID;
	ip : (IP address) if set, filter result for the requested IP address.
	mode: output format ('json' = JSON, 'csv' = CSV TAB-Separated Values, 'sarr' = base64 encoded PHP Serialized array, 'svg' = SVG).

	ADDITIONAL PARAMETERS FOR SVG MODE:

	width: (integer) optional width for SVG output (default 1024; minimum 50).
	height: (integer) optional height for SVG output; will be rounded to a multiple of 5 (default 750, minimum 50).
	scale: linear = vertical linear scale (default), log = vertical logarithmic scale.
	bgcol: type of background color: 'dark' or 'light' (default).
	gtype: sequence of number representing the graphs to display: 1 = CPU TICKS, 2 = IO READ, 3 = IO WRITE, 4 = NET IN, 5 = NET OUT. Default: 12345.

USAGE EXAMPLES:

	JSON:
		serverusage_api.php?from=1332769800&to=1332845100&metric=uid&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=ip&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&mode=json
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&uid=320&mode=json

	CSV:
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&uid=320&mode=psa

	BASE64 ENCODED PHP SERIALIZED ARRAY:
		serverusage_api.php?from=1332769800&to=1332845100&metric=all&uid=320&mode=psa

	SVG:
		serverusage_api.php?from=1332769800&to=1332845100&mode=svg&width=1024&height=750&scale=log
		serverusage_api.php?from=1333532663&to=1333627917&metric=glb&mode=svg&scale=log&bgcol=light&gtype=12345
		serverusage_api.php?from=1333532663&to=1333627917&metric=glb&mode=svg&scale=linear&bgcol=light&gtype=15
		serverusage_api.php?from=1333532663&to=1333627917&metric=glb&mode=svg&scale=log&bgcol=light&gtype=5

OUTPUT:

	The output format can be JSON (JavaScript Object Notation), CSV (tab-separated text values), Base64 encoded serialized array or SVG (Scalable Vector Graphics).

*/

//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
// -----------------------------------------------------------------------------

// READ CONFIGURATION FILE

$conf = parse_ini_file('/etc/serverusage_server.conf');

/**
 * database name (serverusage)
 */
define ('K_DATABASE_NAME', $conf['SQLITE_DATABASE']);

// -----------------------------------------------------------------------------
//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
// -----------------------------------------------------------------------------

// check input parameters

if (!isset($_GET['from']) OR (($from = intval($_GET['from'])) <= 0)) {
	// starting time must be greater than zero
	header('Missing or wrong \'from\' parameter', true, 501);
	exit;
}
if (!isset($_GET['to']) OR (($to = intval($_GET['to'])) < ($from + 60))) {
	// ending time must be at least 60 seconds greater than starting time
	header('Missing or wrong \'to\' parameter', true, 501);
	exit;
}
if (!isset($_GET['mode'])) {
	// default JSON format output
	$mode = 'json';
} elseif (in_array($_GET['mode'], array('json', 'csv', 'psa', 'svg'))) {
	$mode = $_GET['mode'];
} else {
	header('Wrong \'mode\' parameter [\'json\', \'csv\', \'psa\', \'svg\']', true, 501);
	exit;
}
if ($mode == 'svg') {
	// force metric type for SVG output
	$metric = 'glb';
	if (isset($_GET['width'])) {
		$width = max(50, intval($_GET['width']));
	} else {
		// default width
		$width = 1024;
	}
	if (isset($_GET['height'])) {
		// round to a multiple of 5
		$height = (5 * intval(max(50, intval($_GET['height'])) / 5));
	} else {
		// default width
		$height = 750;
	}
	if (isset($_GET['scale']) AND ($_GET['scale'] == 'log')) {
		// print graph in logarithmic scale
		$logscale = true;
	} else {
		$logscale = false;
	}
	if (isset($_GET['bgcol']) AND ($_GET['bgcol']) == 'dark') {
		// dark background color
		$darkbg = true;
		$fcolor = 'ffffff';
		$bcolor = '000000';
	} else {
		$darkbg = false;
		$fcolor = '000000';
		$bcolor = 'ffffff';
	}
	if (isset($_GET['gtype']) AND (intval($_GET['gtype']) > 0)) {
		// select graphs to display
		$gseq = trim($_GET['gtype']);
		$numgraphs = strlen($gseq);
		$gtype = array();
		for ($i = 0; $i < $numgraphs; ++$i) {
			$gtype[] = ($gseq[$i] + 1);
		}
	} else {
		// print all available graphs
		$numgraphs = 5;
		$gtype = array(2,3,4,5,6);
	}
} elseif (isset($_GET['metric']) AND in_array($_GET['metric'], array('uid', 'ip', 'uip', 'grp', 'glb', 'all'))) {
	$metric = $_GET['metric'];
} else {
	header('Wrong \'metric\' parameter [\'uid\', \'ip\', \'uip\', \'grp\', \'glb\', \'all\']', true, 501);
	exit;
}

// -----------------------------------------------------------------------------

// connect to database
try {
	$db = new PDO('sqlite:'.K_DATABASE_NAME);
} catch (PDOException $e) {
	header('SQLite connection error: '.$e->getMessage(), true, 503);
	exit;
}

// create the "where" portion of SQL query
$sqlwhere = ' WHERE lah_start_time>='.$from.' AND lah_end_time<='.$to.'';
if (isset($_GET['uid']) AND (($uid = intval($_GET['uid'])) >= 0)) {
	$sqlwhere .= ' AND lah_user_id='.$uid.'';
}
if (isset($_GET['ip']) AND (preg_match("/^([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})$/", $_GET['ip']) > 0)) {
	$sqlwhere .= ' AND lah_ip=\''.$_GET['ip'].'\'';
}

// select data by metric type
switch ($metric) {
	case 'uid' : {
		$sql = 'SELECT lah_user_id, SUM(lah_cpu_ticks) FROM log_agg_hst'.$sqlwhere.' GROUP BY lah_user_id';
		break;
	}
	case 'ip' : {
		$sql = 'SELECT lah_ip, SUM(lah_netin), SUM(lah_netout) FROM log_agg_hst'.$sqlwhere.' GROUP BY lah_ip';
		break;
	}
	case 'uip' : {
		$sql = 'SELECT lah_user_id, lah_ip, SUM(lah_cpu_ticks), SUM(lah_io_read), SUM(lah_io_write), SUM(lah_netin), SUM(lah_netout) FROM log_agg_hst'.$sqlwhere.' GROUP BY lah_user_id, lah_ip';
		break;
	}
	case 'grp' : {
		$sql = 'SELECT lah_start_time, lah_end_time, lah_user_id, lah_ip, SUM(lah_cpu_ticks), SUM(lah_io_read), SUM(lah_io_write), SUM(lah_netin), SUM(lah_netout) FROM log_agg_hst'.$sqlwhere.' GROUP BY lah_start_time, lah_end_time, lah_user_id, lah_ip ORDER BY lah_end_time';
		break;
	}
	case 'glb' : {
		$sql = 'SELECT lah_start_time, lah_end_time, SUM(lah_cpu_ticks), SUM(lah_io_read), SUM(lah_io_write), SUM(lah_netin), SUM(lah_netout) FROM log_agg_hst'.$sqlwhere.' GROUP BY lah_start_time, lah_end_time ORDER BY lah_start_time';
		break;
	}
	case 'all' : {
		$sql = 'SELECT lah_start_time, lah_end_time, lah_process, lah_user_id, lah_ip, lah_cpu_ticks, lah_io_read, lah_io_write, lah_netin, lah_netout FROM log_agg_hst'.$sqlwhere.' ORDER BY lah_end_time';
		break;
	}
}

// extract requested data
$data = array();
try {
	$r = $db->query($sql, PDO::FETCH_NUM);
	foreach ($r as $row) {
		$data[] = $row;
	}
} catch (PDOException $e) {
	header('SQLite query error: '.$e->getMessage(), true, 503);
	exit;
}

// output data
switch ($mode) {
	case 'csv' : {
		// send headers
		header('Content-Description: CSV Data');
		header('Cache-Control: public, must-revalidate, max-age=0'); // HTTP/1.1
		header('Pragma: public');
		header('Expires: Sat, 26 Jul 1997 05:00:00 GMT'); // Date in the past
		header('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT');
		header('Content-Type: text/csv', false);
		// Turn on output buffering with the gzhandler
		ob_start('ob_gzhandler');
		// output code as CSV tab-separated values
		foreach ($data as $row) {
			echo implode("\t", $row)."\n";
		}
		break;
	}
	case 'psa' : {
		// send headers
		header('Content-Description: Serialized PHP array');
		header('Cache-Control: public, must-revalidate, max-age=0'); // HTTP/1.1
		header('Pragma: public');
		header('Expires: Sat, 26 Jul 1997 05:00:00 GMT'); // Date in the past
		header('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT');
		header('Content-Type: text/plain', false);
		// Turn on output buffering with the gzhandler
		ob_start('ob_gzhandler');
		// output code as base64 encoded PHP serialized array
		echo base64_encode(serialize($data));
		break;
	}
	case 'json' : {
		// send headers
		header('Content-Description: JSON Data');
		header('Cache-Control: public, must-revalidate, max-age=0'); // HTTP/1.1
		header('Pragma: public');
		header('Expires: Sat, 26 Jul 1997 05:00:00 GMT'); // Date in the past
		header('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT');
		header('Content-Type: application/json', false);
		header('Content-Transfer-Encoding: binary');
		// Turn on output buffering with the gzhandler
		ob_start('ob_gzhandler');
		// output code in JSON format
		echo json_encode($data);
		break;
	}
	case 'svg' : {
		// convert array elements to integers and calculate max values
		$maxval = array(0,0,0,0,0,0,0);
		foreach ($data as $k => $v) {
			$data[$k] = array_map('intval', $data[$k]);
			for ($i = 0; $i <=6; ++$i) {
				$maxval[$i] = max($maxval[$i], $data[$k][$i]);
			}
		}
		// colors for lines
		$color = array(2 => 'ab00dc', 3 => '184296', 4 => '589c00', 5 => 'dc7200', 6 => 'b71313');
		// colors for filling area
		$color_fill = array(2 => 'ce26ff', 3 => '2565e9', 4 => '75d000', 5 => 'ff9626', 6 => 'e92525');
		// labels
		$label = array(2 => 'CPU TICKS', 3 => 'IO READ', 4 => 'IO WRITE', 5 => 'NET IN', 6 => 'NET OUT');
		// units
		$unitlabel = array(2 => 'ticks', 3 => 'bytes', 4 => 'bytes', 5 => 'bytes', 6 => 'bytes');
		// horizontal ratio
		$hratio = ($width / ($maxval[1] - $data[0][0]));
		// height of each graph
		$graph_height = round($height / $numgraphs);
		// base for logarithm transformation
		$logbase = array(0,0);
		// vertical scale ratio
		$vratio = array(0,0);
		// initialize line coordinates
		$xs = 0;
		$xe = sprintf('%.3F', (($data[0][1] - $data[0][0]) * $hratio));
		$j = 0;
		for ($i = 2; $i <= 6; ++$i) {
			if (in_array($i, $gtype)) {
				if ($logscale) {
					// logarithm base
					$logbase[$i] = pow(($maxval[$i] + 1), (1.0 / $graph_height));
					// starting point
					$y = sprintf('%.3F', ($height - (($j * $graph_height) + log(($data[0][$i] + 1), $logbase[$i]))));
				} else {
					// vertical ratio
					$vratio[$i] = ($graph_height / $maxval[$i]);
					// starting point
					$y = sprintf('%.3F', ($height - (($j * $graph_height) + ($data[0][$i] * $vratio[$i]))));
				}
				// draw starting points for each column type
				$line[$i] = '<path fill="#'.$color_fill[$i].'" stroke="#'.$color[$i].'" stroke-width="1" d="M 0 '.($height - ($j * $graph_height)).' L '.$xs.' '.$y.' '.$xe.' '.$y.'';
				++$j;
			}
		}
		// create SVG graph
		$svg = '<'.'?'.'xml version="1.0" standalone="no"'.'?'.'>'."\n";
		$svg .= '<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">'."\n";
		$svg .= '<svg width="'.$width.'" height="'.$height.'" version="1.1" xmlns="http://www.w3.org/2000/svg">'."\n";
		$svg .= "\t".'<desc>SERVERUSAGE DATA FROM:'.$from.' TO:'.$to.'</desc>'."\n";
		// count the number of time points
		$numpoints = count($data);
		// draw lines between points
		for ($k = 1; $k < $numpoints; ++$k) {
			$xs = sprintf('%.3F', (($data[$k][0] - $data[0][0]) * $hratio));
			$xe = sprintf('%.3F', (($data[$k][1] - $data[0][0]) * $hratio));
			$j = 0;
			for ($i = 2; $i <= 6; ++$i) {
				if (in_array($i, $gtype)) {
					if ($logscale) {
						$y = sprintf('%.3F', ($height - (($j * $graph_height) + log(($data[$k][$i] + 1), $logbase[$i]))));
					} else {
						$y = sprintf('%.3F', ($height - (($j * $graph_height) + ($data[$k][$i] * $vratio[$i]))));
					}
					$line[$i] .= ' '.$xs.' '.$y.' '.$xe.' '.$y.'';
					++$j;
				}
			}
		}
		// close and print lines
		$j = 0;
		for ($i = 2; $i <= 6; ++$i) {
			if (in_array($i, $gtype)) {
				$svg .= $line[$i].' '.$width.' '.($height - ($j * $graph_height)).' z" />'."\n";
				++$j;
			}
		}
		// display vertical grid (time grid)
		$svg .= '<g stroke="#'.$fcolor.'" stroke-width="1">'."\n";
		for ($k = 1; $k < $numpoints; ++$k) {
			// one line every 10 minutes (600 seconds)
			if (($data[$k][0] % 600) == 0) {
				$x = sprintf('%.3F', (($data[$k][0] - $data[0][0]) * $hratio));
				$svg .= "\t".'<line x1="'.$x.'" y1="0" x2="'.$x.'" y2="'.$height.'"';
				if (($data[$k][0] % 3600) == 0) {
					// mark hour lines
					$svg .= ' opacity="0.3"';
				} else {
					$svg .= ' opacity="0.15"';
				}
				$svg .= ' />'."\n";
			}
		}
		$svg .= '</g>'."\n";
		// horizontal line grids
		$svg .= '<g stroke="#'.$fcolor.'" stroke-width="1">'."\n";
		$j = 0;
		for ($i = 2; $i <= 6; ++$i) {
			if (in_array($i, $gtype)) {
				// line distance
				$ld = pow(10, floor(log(($maxval[$i] / 2), 10)));
				// print lines
				for ($ln = $ld; $ln < $maxval[$i]; $ln += $ld) {
					if ($logscale) {
						// logarithmic
						$y = sprintf('%.3F', ($height - (($j * $graph_height) + log(($ln + 1), $logbase[$i]))));
					} else {
						// linear
						$y = sprintf('%.3F', ($height - (($j * $graph_height) + ($ln * $vratio[$i]))));
					}
					$svg .= "\t".'<line x1="0" y1="'.$y.'" x2="'.$width.'" y2="'.$y.'" opacity="0.3" />'."\n";
				}
				// print a label next to the first line
				$fontsize = round(min(($graph_height / 12),($width / 60)));
				if ($logscale) {
					// logarithmic
					$y = sprintf('%.3F', ($height - (($j * $graph_height) + log(($ld + 1), $logbase[$i]))));
					$x = 5;
				} else {
					// linear
					$y = sprintf('%.3F', ($height - (($j * $graph_height) + ($ld * $vratio[$i]))));
					$x = sprintf('%.3F', ($width / 2));
				}
				$svg .= '<text x="'.$x.'" y="'.($y + $fontsize).'" stroke-width="0" text-anchor="start" font-family="Arial,Verdana" font-size="'.$fontsize.'" fill="#'.$fcolor.'" opacity="0.4">'.number_format($ld,0,'.',',').'</text>'."\n";
				++$j;
			}
		}
		$svg .= '</g>'."\n";
		// add labels
		$svg .= '<g>'."\n";
		$x = ($width / 2);
		$j = 0;
		for ($i = 2; $i <= 6; ++$i) {
			if (in_array($i, $gtype)) {
				$y = ($height - ($j * $graph_height) - ($graph_height / 2));
				$fontsize = round(min(($graph_height / 6),($width / 30)));
				$svg .= '<text x="'.$x.'" y="'.$y.'" opacity="0.7" text-anchor="middle" font-family="Arial,Verdana" font-weight="bold" font-size="'.$fontsize.'" fill="#'.$fcolor.'">'.$label[$i].'</text>'."\n";
				$fontsize = round(min(($graph_height / 9),($width / 50)));
				$svg .= '<text x="'.$x.'" y="'.($y + $fontsize).'" opacity="0.7" text-anchor="middle" font-family="Arial,Verdana" font-size="'.$fontsize.'" fill="#'.$fcolor.'">(MAX: '.number_format($maxval[$i],0,'.',',').' '.$unitlabel[$i].')</text>'."\n";
				++$j;
			}
		}
		// display time
		$y = ($height - 5);
		$fontsize = round(min(($graph_height / 10),($width / 50)));
		$svg .= '<text x="5" y="'.$y.'" opacity="0.6" text-anchor="start" font-family="Arial,Verdana" font-size="'.$fontsize.'" fill="#'.$fcolor.'">'.date('Y-m-d H:i:s', $data[0][0]).'</text>'."\n";
		$svg .= '<text x="'.$width.'" y="'.$y.'" opacity="0.6" text-anchor="end" font-family="Arial,Verdana" font-size="'.$fontsize.'" fill="#'.$fcolor.'">'.date('Y-m-d H:i:s', $maxval[1]).'</text>'."\n";
		$svg .= '</g>'."\n";
		// close SVG graph
		$svg .= '</svg>'."\n";
		// send headers
		header('Content-Description: SVG Data');
		header('Cache-Control: public, must-revalidate, max-age=0'); // HTTP/1.1
		header('Pragma: public');
		header('Expires: Sat, 26 Jul 1997 05:00:00 GMT'); // Date in the past
		header('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT');
		header('Content-Type: image/svg+xml');
		header('Content-Disposition: inline; filename="srvusg.svg";');
		// Turn on output buffering with the gzhandler
		ob_start('ob_gzhandler');
		// output SVG code
		echo $svg;
		break;
	}
}

//=============================================================================+
// END OF FILE
//=============================================================================+
