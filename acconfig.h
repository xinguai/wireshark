/* acconfig.h
 * #ifdefs to be controlled by "configure"
 *
 * $Id: acconfig.h,v 1.28 2003/01/02 20:45:14 guy Exp $
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#undef PACKAGE

#undef VERSION

#undef HAVE_PLUGINS

#undef HAVE_SA_LEN

#undef DATAFILE_DIR

#undef NEED_SNPRINTF_H

#undef NEED_STRERROR_H

#undef NEED_STRPTIME_H

#undef HAVE_LIBPCAP

#undef HAVE_PCAP_VERSION

#undef HAVE_LIBZ

#undef HAVE_NET_SNMP

#undef HAVE_UCD_SNMP

#undef PLUGIN_DIR

@BOTTOM@
#if defined(HAVE_NET_SNMP) || defined(HAVE_UCD_SNMP)
#define HAVE_SOME_SNMP
#endif
