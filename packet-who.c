/* packet-who.c
 * Routines for who protocol (see man rwhod)
 * Gilbert Ramirez <gram@xiexie.org>
 *
 * $Id: packet-who.c,v 1.4 2000/03/12 04:47:51 gram Exp $
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@zing.org>
 * Copyright 1998 Gerald Combs
 *
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#include <time.h>
#include <glib.h>
#include "packet.h"


/*
 *
RWHOD(8)                 UNIX System Manager's Manual                 RWHOD(8)


     The messages sent and received, are of the form:

           struct  outmp {
0                   char    out_line[8];             tty name 
8                   char    out_name[8];             user id 
16                   long    out_time;               time on 
           };

           struct  whod {
 0                   char    wd_vers;
 1                   char    wd_type;
 2                   char    wd_fill[2];
 4                   int     wd_sendtime;
 8                   int     wd_recvtime;
12                   char    wd_hostname[32];
44                   int     wd_loadav[3];
56                   int     wd_boottime;
60                   struct  whoent {
                           struct  outmp we_utmp;
(20 each)                  int     we_idle;
                   } wd_we[1024 / sizeof (struct whoent)];
           };

 Linux 2.0                       May 13, 1997                                2

 *
 */

static int proto_who = -1;
static int hf_who_vers = -1;
static int hf_who_type = -1;
static int hf_who_sendtime = -1;
static int hf_who_recvtime = -1;
static int hf_who_hostname = -1;
static int hf_who_loadav_5 = -1;
static int hf_who_loadav_10 = -1;
static int hf_who_loadav_15 = -1;
static int hf_who_boottime = -1;
static int hf_who_whoent = -1;
static int hf_who_tty = -1;
static int hf_who_uid = -1;
static int hf_who_timeon = -1;
static int hf_who_idle = -1;

static gint ett_who = -1;
static gint ett_whoent = -1;


static void dissect_whoent(const u_char *pd, int offset, frame_data *fd, proto_tree *tree);

void
dissect_who(const u_char *pd, int offset, frame_data *fd, proto_tree *tree)
{

	proto_tree	*who_tree = NULL;
	proto_item	*who_ti = NULL;
	gchar		server_name[33];
	double		loadav_5 = 0.0, loadav_10 = 0.0, loadav_15 = 0.0;

	/* Summary information */
	if (check_col(fd, COL_PROTOCOL))
		col_add_str(fd, COL_PROTOCOL, "WHO");

	/* Figure out if we have enough bytes in the packet
	 * to retrieve the data that we want to put into the summary
	 * line: hostname and load average
	 */
	if ( BYTES_ARE_IN_FRAME(offset, 60) ) {

		memcpy(server_name, &pd[offset + 12], 32);
		server_name[32] = '\0';

		loadav_5  = (double) pntohl(&pd[offset+44]) / 100.0;
		loadav_10 = (double) pntohl(&pd[offset+48]) / 100.0;
		loadav_15 = (double) pntohl(&pd[offset+52]) / 100.0;

		/* Summary information */
		if (check_col(fd, COL_INFO))
			col_add_fstr(fd, COL_INFO, "%s: %.02f %.02f %.02f",
					server_name, loadav_5, loadav_10, loadav_15);
	}
	else {
		return;
	}


	if (tree) {
		struct timeval tv;

		tv.tv_usec = 0;

		/* We already know that the packet has enough data to fill in
		 * the summary info. Retrieve that data */

		who_ti = proto_tree_add_item(tree, proto_who, offset, END_OF_FRAME, NULL);
		who_tree = proto_item_add_subtree(who_ti, ett_who);

		proto_tree_add_item(who_tree, hf_who_vers, offset, 1, pd[offset]);
		offset += 1;


		proto_tree_add_item(who_tree, hf_who_type, offset, 1, pd[offset]);
		offset += 1;

		/* 2 filler bytes */
		offset += 2;

		tv.tv_sec = pntohl(&pd[offset]);
		proto_tree_add_item(who_tree, hf_who_sendtime, offset, 4, &tv);
		offset += 4;

		tv.tv_sec = pntohl(&pd[offset]);
		proto_tree_add_item(who_tree, hf_who_recvtime, offset, 4, &tv);
		offset += 4;

		proto_tree_add_item(who_tree, hf_who_hostname, offset, 32, server_name);
		offset += 32;

		proto_tree_add_item(who_tree, hf_who_loadav_5, offset, 4, loadav_5);
		offset += 4;

		proto_tree_add_item(who_tree, hf_who_loadav_10, offset, 4, loadav_10);
		offset += 4;

		proto_tree_add_item(who_tree, hf_who_loadav_15, offset, 4, loadav_15);
		offset += 4;

		tv.tv_sec = pntohl(&pd[offset]);
		proto_tree_add_item(who_tree, hf_who_boottime, offset, 4, &tv);
		offset += 4;

		dissect_whoent(pd, offset, fd, who_tree);
	}
}

/* The man page says that (1024 / sizeof(struct whoent)) is the maximum number
 * of whoent structures in the packet. */
#define SIZE_OF_WHOENT	24
#define MAX_NUM_WHOENTS	(1024 / SIZE_OF_WHOENT)

static void
dissect_whoent(const u_char *pd, int offset, frame_data *fd, proto_tree *tree)
{
	proto_tree	*whoent_tree = NULL;
	proto_item	*whoent_ti = NULL;
	int		line_offset = offset;
	gchar		out_line[9];	
	gchar		out_name[9];	
	struct timeval	tv;
	int		whoent_num = 0;
	guint32		idle_secs; /* say that out loud... */

	tv.tv_usec = 0;
	out_line[8] = '\0';
	out_name[8] = '\0';

	while (BYTES_ARE_IN_FRAME(line_offset, SIZE_OF_WHOENT) && whoent_num < MAX_NUM_WHOENTS) {
		memcpy(out_line, &pd[line_offset], 8);
		memcpy(out_name, &pd[line_offset+8], 8);

		whoent_ti = proto_tree_add_item(tree, hf_who_whoent, line_offset, SIZE_OF_WHOENT, NULL);
		whoent_tree = proto_item_add_subtree(whoent_ti, ett_whoent);

		proto_tree_add_item(whoent_tree, hf_who_tty, line_offset, 8, out_line);
		line_offset += 8;

		proto_tree_add_item(whoent_tree, hf_who_uid, line_offset, 8, out_name);
		line_offset += 8;

		tv.tv_sec = pntohl(&pd[line_offset]);
		proto_tree_add_item(whoent_tree, hf_who_timeon, line_offset, 4, &tv);
		line_offset += 4;

		idle_secs = pntohl(&pd[line_offset]);
		proto_tree_add_uint_format(whoent_tree, hf_who_idle, line_offset, 4, idle_secs,
				"Idle: %s", time_secs_to_str(idle_secs));
		line_offset += 4;

		whoent_num++;
	}
}

void
proto_register_who(void)
{
        static hf_register_info hf[] = {
                { &hf_who_vers,
                { "Version",	"who.vers", FT_UINT8, BASE_DEC, NULL, 0x0,
			"" }},

                { &hf_who_type,
                { "Type",	"who.type", FT_UINT8, BASE_DEC, NULL, 0x0,
			"" }},

                { &hf_who_sendtime,
                { "Send Time",	"who.sendtime", FT_ABSOLUTE_TIME, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_recvtime,
                { "Receive Time", "who.recvtime", FT_ABSOLUTE_TIME, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_hostname,
                { "Hostname", "who.hostname", FT_STRING, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_loadav_5,
                { "Load Average Over Past  5 Minutes", "who.loadav_5", FT_DOUBLE, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_loadav_10,
                { "Load Average Over Past 10 Minutes", "who.loadav_10", FT_DOUBLE, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_loadav_15,
                { "Load Average Over Past 15 Minutes", "who.loadav_15", FT_DOUBLE, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_boottime,
                { "Boot Time", "who.boottime", FT_ABSOLUTE_TIME, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_whoent,
                { "Who utmp Entry", "who.whoent", FT_NONE, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_tty,
                { "TTY Name", "who.tty", FT_STRING, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_uid,
                { "User ID", "who.uid", FT_STRING, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_timeon,
                { "Time On", "who.timeon", FT_ABSOLUTE_TIME, BASE_NONE, NULL, 0x0,
			"" }},

                { &hf_who_idle,
                { "Time Idle", "who.idle", FT_UINT32, BASE_NONE, NULL, 0x0,
			"" }},
        };

	static gint *ett[] = {
		&ett_who,
		&ett_whoent,
	};

        proto_who = proto_register_protocol("Who", "who");
	proto_register_field_array(proto_who, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}

