/* packet-g723.c
 * Routines for G.723 dissection
 * Copyright 2005, Anders Broman <anders.broman[at]ericsson.com>
 *
 * $Id$
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1998 Gerald Combs
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * References:
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <epan/packet.h>
#include <epan/rtp_pt.h>


/* Initialize the protocol and registered fields */
static int proto_g723			= -1;
static int hf_g723_rate_flag	= -1;
static int hf_g723_vad_flag		= -1;
static int hf_g723_lpc_B5_B0	= -1;

/* Initialize the subtree pointers */
static int ett_g723 = -1;

static const true_false_string g723_rate_flag_vals = {
  "Low rate(5.3kb/s)",
  "High rate(6.3kb/s)"
};
static const true_false_string g723_vad_flag_vals = {
  "Non-speech",
  "Speech"
};


/* Code to actually dissect the packets */
static void
dissect_g723(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	int offset = 0;
	guint octet;

/* Set up structures needed to add the protocol subtree and manage it */
	proto_item *ti;
	proto_tree *g723_tree;

/* Make entries in Protocol column and Info column on summary display */
	if (check_col(pinfo->cinfo, COL_PROTOCOL)) 
		col_set_str(pinfo->cinfo, COL_PROTOCOL, "G.723.1");
	if (tree) {
		ti = proto_tree_add_item(tree, proto_g723, tvb, 0, -1, FALSE);

		g723_tree = proto_item_add_subtree(ti, ett_g723);

		octet = tvb_get_guint8(tvb,offset);
		proto_tree_add_item(g723_tree, hf_g723_rate_flag, tvb, offset, 1, FALSE);
		proto_tree_add_item(g723_tree, hf_g723_vad_flag, tvb, offset, 1, FALSE);
		proto_tree_add_item(g723_tree, hf_g723_lpc_B5_B0, tvb, offset, 1, FALSE);
	
		if ((offset & 0x1) == 1 ) /* Low rate */
			return; 





	}/* if tree */

}


/* Register the protocol with Ethereal */
/* If this dissector uses sub-dissector registration add a registration routine.
   This format is required because a script is used to find these routines and
   create the code that calls these routines.
*/
void
proto_reg_handoff_g723(void)
{
	dissector_handle_t g723_handle;
	
	g723_handle = create_dissector_handle(dissect_g723, proto_g723);

	dissector_add("rtp.pt", PT_G723, g723_handle);

}

/* this format is require because a script is used to build the C function
   that calls all the protocol registration.
*/

void
proto_register_g723(void)
{                 


/* Setup list of header fields  See Section 1.6.1 for details*/
	static hf_register_info hf[] = {
		{ &hf_g723_rate_flag,
			{ "RATEFLAG_B0",           "g723.rate.flag",
			FT_BOOLEAN, 8, TFS(&g723_rate_flag_vals), 0x01,          
			"RATEFLAG_B0", HFILL }
		},
		{ &hf_g723_vad_flag,
			{ "VADFLAG_B0",           "g723.vad.flag",
			FT_BOOLEAN, 8, TFS(&g723_vad_flag_vals), 0x02,          
			"VADFLAG_B0", HFILL }
		},
		{ &hf_g723_lpc_B5_B0,
			{ "LPC_B5...LPC_B0",           "g723.lpc.b5b0",
			FT_UINT8, BASE_HEX, NULL, 0xfc,          
			"LPC_B5...LPC_B0", HFILL }
		},

	};

/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_g723,
	};

/* Register the protocol name and description */
	proto_g723 = proto_register_protocol("G.723","G.723", "g723");

/* Required function calls to register the header fields and subtrees used */
	proto_register_field_array(proto_g723, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));

}


