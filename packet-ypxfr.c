/* packet-ypxfr.c
 * Routines for ypxfr dissection
 *
 * $Id: packet-ypxfr.c,v 1.1 1999/11/10 17:23:55 nneul Exp $
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@unicom.net>
 * Copyright 1998 Gerald Combs
 *
 * Copied from packet-smb.c
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
#include "config.h"
#endif


#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif


#include "packet-rpc.h"
#include "packet-ypxfr.h"

static int proto_ypxfr = -1;

/* proc number, "proc name", dissect_request, dissect_reply */
/* NULL as function pointer means: take the generic one. */
const vsff ypxfr1_proc[] = {
	{ YPXFRPROC_NULL,	"NULL",		NULL,				NULL },
	{ YPXFRPROC_GETMAP,	"GETMAP",		NULL,				NULL },
	{ 0,	NULL,		NULL,				NULL }
};
/* end of YPXFR version 1 */

void
proto_register_ypxfr(void)
{
	proto_ypxfr = proto_register_protocol("Yellow Pages Transfer", "ypxfr");

	/* Register the protocol as RPC */
	rpc_init_prog(proto_ypxfr, YPXFR_PROGRAM, ETT_YPXFR);
	/* Register the procedure tables */
	rpc_init_proc_table(YPXFR_PROGRAM, 1, ypxfr1_proc);
}

