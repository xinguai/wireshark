/* packet-uccreq.c
 * Routines for Upstream Channel Change Request dissection
 * Copyright 2002, Anand V. Narwani <anarwani@cisco.com>
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
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#include "plugins/plugin_api.h"
#include "plugins/plugin_api_defs.h"
#include "moduleinfo.h"

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include <gmodule.h>

#ifdef NEED_SNPRINTF_H
# include "snprintf.h"
#endif

#include <epan/packet.h>

/* Initialize the protocol and registered fields */
static int proto_docsis_uccreq = -1;
static int hf_docsis_uccreq = -1;
static int hf_docsis_uccreq_upchid = -1;
static dissector_handle_t docsis_tlv_handle;


/* Initialize the subtree pointers */
static gint ett_docsis_uccreq = -1;

/* Code to actually dissect the packets */
static void
dissect_uccreq (tvbuff_t * tvb, packet_info * pinfo, proto_tree * tree)
{

  proto_item *it;
  proto_tree *uccreq_tree;
  guint16 tlv_data_len;
  guint8 chid;
  tvbuff_t *next_tvb;

  chid = tvb_get_guint8 (tvb, 0);
  tlv_data_len = tvb_length_remaining (tvb, 1);

  if (check_col (pinfo->cinfo, COL_INFO))
    {
      col_clear (pinfo->cinfo, COL_INFO);
      col_add_fstr (pinfo->cinfo, COL_INFO,
		    "Upstream Channel Change request  Channel ID = %u (U%u)",
		    chid, (chid > 0 ? chid - 1 : chid));
    }

  if (tree)
    {
      it =
	proto_tree_add_protocol_format (tree, proto_docsis_uccreq, tvb, 0,
					tvb_length_remaining (tvb, 0),
					"UCC Request");
      uccreq_tree = proto_item_add_subtree (it, ett_docsis_uccreq);
      proto_tree_add_item (uccreq_tree, hf_docsis_uccreq_upchid, tvb, 0, 1,
			   FALSE);

      /* call dissector for Appendix C TLV's */
      next_tvb = tvb_new_subset (tvb, 1, tlv_data_len, tlv_data_len);
      call_dissector (docsis_tlv_handle, next_tvb, pinfo, uccreq_tree);
    }


}




/* Register the protocol with Ethereal */

/* this format is require because a script is used to build the C function
   that calls all the protocol registration.
*/


void
proto_register_docsis_uccreq (void)
{

/* Setup list of header fields  See Section 1.6.1 for details*/
  static hf_register_info hf[] = {
    {&hf_docsis_uccreq,
     {"Upstream Channel Change Request", "docsis.uccreq",
      FT_BYTES, BASE_HEX, NULL, 0x0,
      "Upstream Channel Change Request", HFILL}
     },
    {&hf_docsis_uccreq_upchid,
     {"Upstream Channel Id", "docsis.uccreq.upchid",
      FT_UINT8, BASE_DEC, NULL, 0x0,
      "Upstream Channel Id", HFILL}
     },
  };

/* Setup protocol subtree array */
  static gint *ett[] = {
    &ett_docsis_uccreq,
  };

/* Register the protocol name and description */
  proto_docsis_uccreq =
    proto_register_protocol ("DOCSIS Upstream Channel Change Request",
			     "DOCSIS UCC-REQ", "docsis_uccreq");

/* Required function calls to register the header fields and subtrees used */
  proto_register_field_array (proto_docsis_uccreq, hf, array_length (hf));
  proto_register_subtree_array (ett, array_length (ett));

  register_dissector ("docsis_uccreq", dissect_uccreq, proto_docsis_uccreq);
}


/* If this dissector uses sub-dissector registration add a registration routine.
   This format is required because a script is used to find these routines and
   create the code that calls these routines.
*/
void
proto_reg_handoff_docsis_uccreq (void)
{
  dissector_handle_t docsis_uccreq_handle;

  docsis_uccreq_handle = find_dissector ("docsis_uccreq");
  docsis_tlv_handle = find_dissector ("docsis_tlv");
  dissector_add ("docsis_mgmt", 0x08, docsis_uccreq_handle);

}
