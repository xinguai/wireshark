/* alert_box.h
 * Routines to put up various "standard" alert boxes used in multiple
 * places
 *
 * $Id: alert_box.h,v 1.1 2004/02/11 00:55:27 guy Exp $
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

#ifndef __ALERT_BOX_H__
#define __ALERT_BOX_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Alert box for an invalid display filter expression.
 * Assumes "dfilter_error_msg" has been set by "dfilter_compile()" to the
 * error message for the filter.
 */
extern void bad_dfilter_alert_box(const char *dftext);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ALERT_BOX_H__ */
