/* ethereal.h
 * Global defines, etc.
 *
 * $Id: ethereal.h,v 1.12 1999/03/23 03:14:33 gram Exp $
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

#ifndef __ETHEREAL_H__
#define __ETHEREAL_H__

#include "config.h"

#define PF_DIR ".ethereal"
#define RC_FILE PF_DIR "/gtkrc"
#define MONO_MEDIUM_FONT "-*-lucidatypewriter-medium-r-normal-*-*-120-*-*-*-*-iso8859-1"
#define MONO_BOLD_FONT "-*-lucidatypewriter-bold-r-normal-*-*-120-*-*-*-*-iso8859-1"
#define DEF_WIDTH 750
#define DEF_HEIGHT 550
#define DEF_READY_MESSAGE " Ready to load or capture"
#define EXTERNAL_FILTER "/usr/local/bin/ethereal_tcp_filter -f" 

#define MIN_PACKET_SIZE 68	/* minimum amount of packet data we can read */
#define MAX_PACKET_SIZE 65535	/* maximum amount of packet data we can read */

/* Byte swapping routines */
#define SWAP16(x) \
  ( (((x) & 0x00ff) << 8) | \
    (((x) & 0xff00) >> 8) )
#define SWAP32(x) \
  ( (((x) & 0x000000ff) << 24) | \
    (((x) & 0x0000ff00) <<  8) | \
    (((x) & 0x00ff0000) >>  8) | \
    (((x) & 0xff000000) >> 24) )

/* Byte ordering */
#ifndef BYTE_ORDER
  #define LITTLE_ENDIAN 4321
  #define BIG_ENDIAN 1234
  #ifdef WORDS_BIGENDIAN
    #define BYTE_ORDER BIG_ENDIAN
  #else
    #define BYTE_ORDER LITTLE_ENDIAN
  #endif
#endif

/* From the K&R book, p. 89 */
#ifndef MAX
  #define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
  #define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

/* Determine whether we use menu factories or item factories. This
 * code snippet is taken from cheops.h of cheops-0.57, a GPL'ed
 * network utility program Copyright (C) 1998, Mark Spencer
 */
#if (GTK_MINOR_VERSION > 1) || ((GTK_MICRO_VERSION > 1) &&  (GTK_MINOR_VERSION > 0))
	#define USE_ITEM
	#define GTK_MENU_FUNC(a) ((GtkItemFactoryCallback)(a))
#else
	#undef USE_ITEM
	typedef void (*_GTK_MENU_FUNC_T)(GtkWidget *, void *);
	#define GTK_MENU_FUNC(a) ((_GTK_MENU_FUNC_T)(a))
#endif

    
typedef struct _selection_info {
  GtkWidget *tree;
  GtkWidget *text;
} selection_info;


/*
 * Type of time-stamp shown in the summary display.
 */
typedef enum {
	RELATIVE,
	ABSOLUTE,
	DELTA
} ts_type;

extern ts_type timestamp_type;

extern GtkStyle *item_style;

void about_ethereal( GtkWidget *, gpointer);
void file_sel_ok_cb(GtkWidget *, GtkFileSelection *);
void blank_packetinfo();
gint file_progress_cb(gpointer);
void follow_stream_cb( GtkWidget *, gpointer);
void file_open_cmd_cb(GtkWidget *, gpointer);
void file_close_cmd_cb(GtkWidget *, gpointer);
void file_quit_cmd_cb(GtkWidget *, gpointer);
void file_reload_cmd_cb(GtkWidget *, gpointer);
void file_print_cmd_cb(GtkWidget *, gpointer);
void main_realize_cb(GtkWidget *, gpointer);

#endif /* ethereal.h */
