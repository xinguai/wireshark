/* export_object.c
 * Common routines for tracking & saving objects found in streams of data
 * Copyright 2007, Stephen Fisher <stephentfisher@yahoo.com>
 *
 * $Id$
 * 
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>

/* This feature has not been ported to GTK1 */
#if GTK_MAJOR_VERSION >= 2

#include <alert_box.h>
#include <simple_dialog.h>

#include <epan/packet_info.h>
#include <epan/prefs.h>
#include <epan/tap.h>
#include <gtk/compat_macros.h>
#include <gtk/dlg_utils.h>
#include <gtk/file_dlg.h>
#include <gtk/gui_utils.h>
#include <gtk/help_dlg.h>
#include <gtk/main.h>
#include <wiretap/file_util.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "export_object.h"

enum {
	EO_PKT_NUM_COLUMN,
	EO_HOSTNAME_COLUMN,
	EO_CONTENT_TYPE_COLUMN,
	EO_BYTES_COLUMN,
	EO_FILENAME_COLUMN,
	EO_NUM_COLUMNS /* must be last */
};


static void
eo_remember_this_row(GtkTreeModel *model _U_, GtkTreePath *path,
		     GtkTreeIter *iter _U_, gpointer arg)
{
	export_object_list_t *object_list = arg;
	export_object_entry_t *entry;

	gint *path_index;

	if((path_index = gtk_tree_path_get_indices(path)) == NULL)
		return;

	object_list->row_selected = path_index[0];

	entry = g_slist_nth_data(object_list->entries,
				 object_list->row_selected);
       
	cf_goto_frame(&cfile, entry->pkt_num);
}

static void
eo_remember_row_num(GtkTreeSelection *sel, gpointer data)
{
	gtk_tree_selection_selected_foreach(sel, eo_remember_this_row, data);
}


static void
eo_win_destroy_cb(GtkWindow *win _U_, gpointer data)
{
        export_object_list_t *object_list = data;

        protect_thread_critical_region();
        remove_tap_listener(object_list);
        unprotect_thread_critical_region();

   	g_free(object_list);
}

static void
eo_save_entry(gchar *save_as_filename, export_object_entry_t *entry)
{
	int to_fd;

	to_fd = eth_open(save_as_filename, O_WRONLY | O_CREAT | O_EXCL |
			 O_BINARY, 0644);
	if(to_fd == -1) { /* An error occurred */
		open_failure_alert_box(save_as_filename, errno, TRUE);
		g_free(save_as_filename);
		return;
	}

	if(eth_write(to_fd, entry->payload_data, entry->payload_len) < 0) {
		write_failure_alert_box(save_as_filename, errno);
		eth_close(to_fd);
		g_free(save_as_filename);
		return;
	}

	if (eth_close(to_fd) < 0) {
		write_failure_alert_box(save_as_filename, errno);
		g_free(save_as_filename);
		return;
	}

	g_free(save_as_filename);
}


static void
eo_save_clicked_cb(GtkWidget *widget _U_, gpointer arg)
{
	GtkWidget *save_as_w;
	export_object_list_t *object_list = arg;
	export_object_entry_t *entry = NULL;

	entry = g_slist_nth_data(object_list->entries,
				 object_list->row_selected);

	if(!entry) {
		simple_dialog(ESD_TYPE_WARN, ESD_BTN_OK, "No object was selected for saving.  Please click on an object and click save again.");
		return;
	}

	save_as_w = file_selection_new("Wireshark: Save Object As ...",
				       FILE_SELECTION_SAVE);

	gtk_window_set_transient_for(GTK_WINDOW(save_as_w),
				     GTK_WINDOW(object_list->dlg));

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_as_w),
					  entry->filename);

	if(gtk_dialog_run(GTK_DIALOG(save_as_w)) == GTK_RESPONSE_ACCEPT)
		eo_save_entry(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_as_w)), entry);

	window_destroy(save_as_w);
}

static void
eo_save_all_clicked_cb(GtkWidget *widget _U_, gpointer arg)
{
	gchar *save_as_fullpath;
	export_object_list_t *object_list = arg;
	export_object_entry_t *entry;
	GtkWidget *save_in_w;
	GSList *last_slist_entry;
	gint last_slist_entry_num, i;

	save_in_w = file_selection_new("Wireshark: Save All Objects In ...",
				       FILE_SELECTION_CREATE_FOLDER);

	gtk_window_set_transient_for(GTK_WINDOW(save_in_w),
				     GTK_WINDOW(object_list->dlg));

	if(gtk_dialog_run(GTK_DIALOG(save_in_w)) == GTK_RESPONSE_ACCEPT) {

		/* Find the last entry in the SList, then start at the beginning
		   saving each one until we reach the last entry. */
		last_slist_entry = g_slist_last(object_list->entries);
		last_slist_entry_num = g_slist_position(object_list->entries,
							last_slist_entry);

		for(i = 0; i <= last_slist_entry_num; i++) {
			
			entry = g_slist_nth_data(object_list->entries, i);

			save_as_fullpath = g_build_filename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_in_w)), entry->filename, NULL);
			
			eo_save_entry(save_as_fullpath, entry);
		}
	}

	window_destroy(save_in_w);
}

static void
eo_reset(void *tapdata)
{
	export_object_list_t *object_list = tapdata;

	if(object_list->entries) {
		g_slist_free(object_list->entries);
		object_list->entries = NULL;
	}

	object_list->iter = NULL;
	object_list->row_selected = -1;
}

static void
eo_draw(void *tapdata)
{
	export_object_list_t *object_list = tapdata;
	export_object_entry_t *eo_entry;

	GSList *slist_entry = NULL;
	GSList *last_slist_entry = NULL;
	gint last_slist_entry_num;
	GtkTreeIter new_iter;
	gchar *column_text[EO_NUM_COLUMNS];

	last_slist_entry = g_slist_last(object_list->entries);
	last_slist_entry_num = g_slist_position(object_list->entries,
						last_slist_entry);

	while(object_list->slist_pos <= last_slist_entry_num &&
	      last_slist_entry_num != -1) {
		slist_entry = g_slist_nth(object_list->entries,
					  object_list->slist_pos);
		eo_entry = slist_entry->data;
		
		column_text[0] = g_strdup_printf("%u", eo_entry->pkt_num);
		column_text[1] = g_strdup_printf("%s", eo_entry->hostname);
		column_text[2] = g_strdup_printf("%s", eo_entry->content_type);
		column_text[3] = g_strdup_printf("%u", eo_entry->payload_len);
		column_text[4] = g_strdup_printf("%s", eo_entry->filename);

		gtk_tree_store_append(object_list->store, &new_iter,
				      object_list->iter);

		gtk_tree_store_set(object_list->store, &new_iter,
				   EO_PKT_NUM_COLUMN, column_text[0],
				   EO_HOSTNAME_COLUMN, column_text[1],
				   EO_CONTENT_TYPE_COLUMN, column_text[2],
				   EO_BYTES_COLUMN, column_text[3],
				   EO_FILENAME_COLUMN, column_text[4],
				   -1);

		g_free(column_text[0]);
		g_free(column_text[1]);
		g_free(column_text[2]);
		g_free(column_text[3]);
		g_free(column_text[4]);

		object_list->slist_pos++;
	}
}

void
export_object_window(gchar *tapname, tap_packet_cb tap_packet)
{
	GtkWidget *sw;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	GtkWidget *vbox, *bbox, *help_bt, *close_bt, *save_bt, *save_all_bt;
	GtkTooltips *button_bar_tips;
	GString *error_msg;
	export_object_list_t *object_list;
	gchar *window_title;

	/* Initialize our object list structure */
	object_list = g_malloc0(sizeof(export_object_list_t));

	/* Data will be gathered via a tap callback */
	error_msg = register_tap_listener(tapname, object_list, NULL,
					  eo_reset,
					  tap_packet,
					  eo_draw);

	if (error_msg) {
		simple_dialog(ESD_TYPE_ERROR, ESD_BTN_OK,
			      "Can't register http tap: %s\n", error_msg->str);
		g_string_free(error_msg, TRUE);
		return;
	}

	/* Set up our GUI window */
	button_bar_tips = gtk_tooltips_new();

	window_title = g_strdup_printf("Wireshark: %s object list", tapname);
	object_list->dlg = dlg_window_new(window_title);
	g_free(window_title);

	gtk_window_set_default_size(GTK_WINDOW(object_list->dlg),
				    DEF_WIDTH, DEF_HEIGHT);

	vbox = gtk_vbox_new(FALSE, 5);

        gtk_container_border_width(GTK_CONTAINER(vbox), 5);
        gtk_container_add(GTK_CONTAINER(object_list->dlg), vbox);

	sw = scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
					    GTK_SHADOW_IN);

	gtk_container_add(GTK_CONTAINER(vbox), sw);

	object_list->store = gtk_tree_store_new(EO_NUM_COLUMNS,
						 G_TYPE_STRING, G_TYPE_STRING,
						 G_TYPE_STRING, G_TYPE_STRING,
						 G_TYPE_STRING);

	object_list->tree = tree_view_new(GTK_TREE_MODEL(object_list->store));

	object_list->tree_view = GTK_TREE_VIEW(object_list->tree);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Packet num",
							  renderer,
							  "text",
							  EO_PKT_NUM_COLUMN,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column(object_list->tree_view, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Hostname",
							  renderer,
							  "text",
							  EO_HOSTNAME_COLUMN,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column(object_list->tree_view, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Content Type",
							  renderer,
							  "text",
							  EO_CONTENT_TYPE_COLUMN,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column(object_list->tree_view, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Bytes",
							  renderer,
							  "text",
							  EO_BYTES_COLUMN,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column(object_list->tree_view, column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Filename",
							  renderer,
							  "text",
							  EO_FILENAME_COLUMN,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column(object_list->tree_view, column);

	gtk_container_add(GTK_CONTAINER(sw), object_list->tree);

	selection = gtk_tree_view_get_selection(object_list->tree_view);
        SIGNAL_CONNECT(selection, "changed", eo_remember_row_num, object_list);


 	bbox = gtk_hbox_new(FALSE, 5);

	/* Help button */
	help_bt = BUTTON_NEW_FROM_STOCK(GTK_STOCK_HELP);
	SIGNAL_CONNECT(help_bt, "clicked", topic_cb, HELP_EXPORT_OBJECT_LIST);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(button_bar_tips), help_bt,
			     "Show help on this dialog", NULL);
	gtk_box_pack_start(GTK_BOX(bbox), help_bt, FALSE, FALSE, 0);

	/* Save All button */
	save_all_bt = gtk_button_new_with_mnemonic("Save _All");
	SIGNAL_CONNECT(save_all_bt, "clicked", eo_save_all_clicked_cb,
		       object_list);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(button_bar_tips), save_all_bt,
			     "Save all listed objects with their displayed "
			     "filenames.", NULL);
	gtk_box_pack_end(GTK_BOX(bbox), save_all_bt, FALSE, FALSE, 0);

	/* Save button */
	save_bt = BUTTON_NEW_FROM_STOCK(GTK_STOCK_SAVE_AS);
	SIGNAL_CONNECT(save_bt, "clicked", eo_save_clicked_cb, object_list);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(button_bar_tips), save_bt,
			     "Saves the currently selected content to a file.",
			     NULL);
	gtk_box_pack_end(GTK_BOX(bbox), save_bt, FALSE, FALSE, 0);

	/* Close button */
        close_bt = BUTTON_NEW_FROM_STOCK(GTK_STOCK_CLOSE);
        GTK_WIDGET_SET_FLAGS(close_bt, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(button_bar_tips), close_bt,
			     "Close this dialog.", NULL);
	gtk_box_pack_end(GTK_BOX(bbox), close_bt, FALSE, FALSE, 0);

	/* Pack the buttons into the "button box" */
        gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
        gtk_widget_show(bbox);

	/* Setup cancel/delete/destroy signal handlers */
        SIGNAL_CONNECT(object_list->dlg, "delete_event",
		       window_delete_event_cb, NULL);
	SIGNAL_CONNECT(object_list->dlg, "destroy",
		       eo_win_destroy_cb, NULL);
       	window_set_cancel_button(object_list->dlg, close_bt,
				 window_cancel_button_cb);

	/* Show the window */
	gtk_widget_show_all(object_list->dlg);
	window_present(object_list->dlg);

	cf_retap_packets(&cfile, FALSE);
}

#endif /* GTK_MAJOR_VERSION >= 2 */
